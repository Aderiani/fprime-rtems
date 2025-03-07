// ======================================================================
// \title Os/RTEMS/File.cpp
// \brief RTEMS implementation for Os::File
// ======================================================================
#include <Os/File.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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
    Status open(const char* filename, Mode mode) override {
        FW_ASSERT(filename != nullptr);
        
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

        m_handle.fd = ::open(filename, flags, 0644);
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

    //! Read from file into buffer
    Status read(U8* buffer, PlatformSizeType& size, WaitType wait) override {
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
        
        size = static_cast<PlatformSizeType>(read_size);
        return Status::OP_OK;
    }

    //! Read line from file
    Status readline(U8* buffer, PlatformSizeType& size, WaitType wait) override {
        FW_ASSERT(buffer != nullptr);
        
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        PlatformSizeType offset = 0;
        U8 byte;
        PlatformSizeType byte_size = 1;

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
    Status write(const U8* buffer, PlatformSizeType& size) override {
        FW_ASSERT(buffer != nullptr);
        
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        ssize_t write_size = ::write(m_handle.fd, buffer, size);
        if (write_size < 0) {
            if (errno == ENOSPC) {
                return Status::NO_SPACE;
            }
            return Status::OTHER_ERROR;
        }
        
        size = static_cast<PlatformSizeType>(write_size);
        return Status::OP_OK;
    }

    //! Seek to position in file
    Status seek(PlatformSizeType offset, SeekType seekType) override {
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
    Status close() override {
        if (m_handle.fd == -1) {
            return Status::NOT_OPENED;
        }

        if (::close(m_handle.fd) == -1) {
            return Status::OTHER_ERROR;
        }
        
        m_handle.fd = -1;
        return Status::OP_OK;
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
FileInterface* FileInterface::getDelegate(FileHandleStorage& aligned_new_memory) {
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::File::RtemsFile) <= sizeof(FileHandleStorage),
                  "RTEMS file implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::File::RtemsFile)) == 0,
                  "Bad alignment for RTEMS file implementation");
    return new (aligned_new_memory) Os::RTEMS::File::RtemsFile;
}
}