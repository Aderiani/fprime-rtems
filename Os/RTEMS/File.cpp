// ======================================================================
// \title Os/RTEMS/File.cpp
// \brief RTEMS implementation for Os::File
// ======================================================================
#include <Os/File.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace Os {
namespace RTEMS {
namespace File {

struct RtemsFileHandle : public FileHandle {
    int fd; // File descriptor
    RtemsFileHandle() : fd(-1) {}
};

class RtemsFile : public FileInterface {
  public:
    //! Constructor
    RtemsFile() {
        m_handle.fd = -1;
    }

    //! Destructor
    ~RtemsFile() override {
        if (m_handle.fd != -1) {
            this->close();
        }
    }

    //! Open file with the specified mode
    Status open(const char* path, Mode mode, OverwriteType overwrite) override {
        FW_ASSERT(path != nullptr);
        
        if (m_handle.fd != -1) {
            return Status::OP_OK;
        }

        int flags = 0;
        switch (mode) {
            case Mode::OPEN_READ:
                flags = O_RDONLY;
                break;
            case Mode::OPEN_WRITE:
                flags = O_WRONLY | O_CREAT;
                break;
            case Mode::OPEN_SYNC_WRITE:
                flags = O_WRONLY | O_CREAT | O_SYNC;
                break;
            case Mode::OPEN_CREATE:
                flags = O_WRONLY | O_CREAT | O_TRUNC;
                break;
            case Mode::OPEN_APPEND:
                flags = O_WRONLY | O_CREAT | O_APPEND;
                break;
            default:
                return Status::INVALID_MODE;
        }
        
        // Apply overwrite flag
        if (overwrite == OverwriteType::NO_OVERWRITE) {
            flags |= O_EXCL;  // Fail if file exists
        }

        m_handle.fd = ::open(path, flags, 0644);
        if (m_handle.fd == -1) {
            switch (errno) {
                case ENOENT:
                    return Status::DOESNT_EXIST;
                case EACCES:
                    return Status::NO_PERMISSION;
                case EISDIR:
                    return Status::NOT_SUPPORTED;
                case EEXIST:
                    return Status::FILE_EXISTS;
                default:
                    return Status::OTHER_ERROR;
            }
        }
        return Status::OP_OK;
    }

    //! Get file size
    Status size(FwSignedSizeType& size_result) override {
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }
        
        struct stat st;
        if (fstat(m_handle.fd, &st) != 0) {
            return Status::OTHER_ERROR;
        }
        
        size_result = static_cast<FwSignedSizeType>(st.st_size);
        return Status::OP_OK;
    }

    //! Get current file position
    Status position(FwSignedSizeType& position_result) override {
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }
        
        off_t pos = lseek(m_handle.fd, 0, SEEK_CUR);
        if (pos == -1) {
            return Status::OTHER_ERROR;
        }
        
        position_result = static_cast<FwSignedSizeType>(pos);
        return Status::OP_OK;
    }

    //! Preallocate space in the file
    Status preallocate(FwSignedSizeType offset, FwSignedSizeType length) override {
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }
        
        // RTEMS might not support posix_fallocate, so we'll use a fallback approach
        // by extending the file with lseek/write if needed
        
        FwSignedSizeType current_size;
        Status status = this->size(current_size);
        if (status != Status::OP_OK) {
            return status;
        }
        
        // If requested size is already allocated, we're done
        if (offset + length <= current_size) {
            return Status::OP_OK;
        }
        
        // Otherwise, extend the file by seeking to the end position and writing a byte
        off_t current_pos = lseek(m_handle.fd, 0, SEEK_CUR); // Save current position
        if (current_pos == -1) {
            return Status::OTHER_ERROR;
        }
        
        // Seek to the end position
        if (lseek(m_handle.fd, offset + length - 1, SEEK_SET) == -1) {
            return Status::OTHER_ERROR;
        }
        
        // Write a byte to extend the file - use system call directly
        char zero = 0;
        if (::write(m_handle.fd, &zero, 1) != 1) {
            // Restore original position on error
            lseek(m_handle.fd, current_pos, SEEK_SET);
            return Status::OTHER_ERROR;
        }
        
        // Restore original position
        if (lseek(m_handle.fd, current_pos, SEEK_SET) == -1) {
            return Status::OTHER_ERROR;
        }
        
        return Status::OP_OK;
    }

    //! Seek to position in file
    Status seek(FwSignedSizeType offset, SeekType seekType) override {
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        int whence;
        switch (seekType) {
            case SeekType::ABSOLUTE:
                whence = SEEK_SET;
                break;
            case SeekType::RELATIVE:
                whence = SEEK_CUR;
                break;
            default:
                return Status::INVALID_ARGUMENT;
        }

        if (::lseek(m_handle.fd, static_cast<off_t>(offset), whence) == -1) {
            return Status::OTHER_ERROR;
        }
        
        return Status::OP_OK;
    }

    //! Read from file into buffer
    Status read(U8* buffer, FwSignedSizeType& size, WaitType wait) override {
        FW_ASSERT(buffer != nullptr);
        
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        ssize_t read_size = ::read(m_handle.fd, buffer, size);
        if (read_size < 0) {
            if (errno == EAGAIN && wait == WaitType::NO_WAIT) {
                size = 0;
                return Status::OP_OK;
            }
            return Status::OTHER_ERROR;
        }
        
        size = static_cast<FwSignedSizeType>(read_size);
        return Status::OP_OK;
    }

    //! Read line from file - implementation for convenience but not part of interface
    Status readline(U8* buffer, FwSignedSizeType& size, WaitType wait) {
        FW_ASSERT(buffer != nullptr);
        
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        FwSignedSizeType offset = 0;
        U8 byte;
        FwSignedSizeType byte_size = 1;

        while (offset < size - 1) {  // Leave room for null terminator
            Status read_status = this->read(&byte, byte_size, wait);
            if (read_status != Status::OP_OK) {
                return read_status;
            }
            
            if (byte_size == 0) {  // No data available
                if (offset == 0) {
                    return Status::OTHER_ERROR;
                }
                break;
            }
            
            buffer[offset++] = byte;
            
            if (byte == '\n') {
                break;
            }
        }
        
        buffer[offset] = '\0';
        size = offset;
        
        return Status::OP_OK;
    }

    //! Write to file
    Status write(const U8* buffer, FwSignedSizeType& size, WaitType wait) override {
        FW_ASSERT(buffer != nullptr);
        
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        ssize_t write_size = ::write(m_handle.fd, buffer, size);
        if (write_size < 0) {
            if (errno == ENOSPC) {
                return Status::NO_SPACE;
            }
            if (errno == EAGAIN && wait == WaitType::NO_WAIT) {
                size = 0;
                return Status::OP_OK;
            }
            return Status::OTHER_ERROR;
        }
        
        size = static_cast<FwSignedSizeType>(write_size);
        return Status::OP_OK;
    }

    //! Flush file buffers
    Status flush() override {
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        if (::fsync(m_handle.fd) == -1) {
            return Status::OTHER_ERROR;
        }
        
        return Status::OP_OK;
    }

    //! Close file
    void close() override {
        if (m_handle.fd != -1) {
            ::close(m_handle.fd);
            m_handle.fd = -1;
        }
    }

    //! Get file handle
    FileHandle* getHandle() override {
        return &m_handle;
    }

  private:
    RtemsFileHandle m_handle;
};

} // namespace File
} // namespace RTEMS
} // namespace Os

namespace Os {
FileInterface* FileInterface::getDelegate(FileHandleStorage& aligned_new_memory, const FileInterface* to_copy) {
    // References cannot be null, so we can safely remove this assert
    static_assert(sizeof(Os::RTEMS::File::RtemsFile) <= sizeof(FileHandleStorage),
                  "RTEMS file implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::File::RtemsFile)) == 0,
                  "Bad alignment for RTEMS file implementation");
    return new (aligned_new_memory) Os::RTEMS::File::RtemsFile();
}
} // namespace Os   