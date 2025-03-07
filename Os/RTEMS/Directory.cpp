// ======================================================================
// \title Os/RTEMS/Directory.cpp
// \brief RTEMS implementation for Os::Directory
// ======================================================================
#include <Os/Directory.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

namespace Os {
namespace RTEMS {
namespace Directory {

struct RtemsDirectoryHandle : public DirectoryHandle {
    DIR* dir;
    RtemsDirectoryHandle() : dir(nullptr) {}
};

class RtemsDirectory : public DirectoryInterface {
  public:
    //! Constructor
    RtemsDirectory() {
        m_handle.dir = nullptr;
    }

    //! Destructor
    ~RtemsDirectory() override {
        if (m_handle.dir != nullptr) {
            this->close();
        }
    }

    //! Open directory
    Status open(const char* path, OpenMode mode) override {
        FW_ASSERT(path != nullptr);
        
        if (m_handle.dir != nullptr) {
            return Status::OP_OK;
        }

        struct stat st;
        if (stat(path, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                return Status::NOT_DIR;
            }
            
            if (mode == OpenMode::CREATE_EXCLUSIVE) {
                return Status::ALREADY_EXISTS;
            }
            
            m_handle.dir = opendir(path);
            if (m_handle.dir == nullptr) {
                if (errno == EACCES) {
                    return Status::NO_PERMISSION;
                }
                return Status::OTHER_ERROR;
            }
            
            return Status::OP_OK;
        } else {
            if (errno == ENOENT) {
                if (mode == OpenMode::READ) {
                    return Status::DOESNT_EXIST;
                }
                
                // Try to create the directory
                if (mkdir(path, 0755) != 0) {
                    if (errno == EACCES) {
                        return Status::NO_PERMISSION;
                    }
                    return Status::OTHER_ERROR;
                }
                
                m_handle.dir = opendir(path);
                if (m_handle.dir == nullptr) {
                    return Status::OTHER_ERROR;
                }
                
                return Status::OP_OK;
            }
            
            return Status::OTHER_ERROR;
        }
    }

    //! Read next entry in directory
    Status read(char* entry, PlatformSizeType maxSize) override {
        FW_ASSERT(entry != nullptr);
        
        if (m_handle.dir == nullptr) {
            return Status::NOT_OPENED;
        }
        
        errno = 0;
        struct dirent* dirent = readdir(m_handle.dir);
        
        if (dirent == nullptr) {
            if (errno != 0) {
                return Status::OTHER_ERROR;
            }
            return Status::NO_MORE_FILES;
        }
        
        PlatformSizeType nameLen = strlen(dirent->d_name);
        if (nameLen >= maxSize) {
            return Status::FILE_LIMIT;
        }
        
        strncpy(entry, dirent->d_name, maxSize);
        return Status::OP_OK;
    }

    //! Remove directory
    Status remove(const char* path) override {
        FW_ASSERT(path != nullptr);
        
        if (rmdir(path) != 0) {
            switch (errno) {
                case ENOENT:
                    return Status::DOESNT_EXIST;
                case EACCES:
                    return Status::NO_PERMISSION;
                case ENOTDIR:
                    return Status::NOT_DIR;
                case ENOTEMPTY:
                    return Status::OTHER_ERROR;
                default:
                    return Status::OTHER_ERROR;
            }
        }
        
        return Status::OP_OK;
    }

    //! Reset directory read position to beginning
    Status rewind() override {
        if (m_handle.dir == nullptr) {
            return Status::NOT_OPENED;
        }
        
        rewinddir(m_handle.dir);
        return Status::OP_OK;
    }

    //! Close directory
    Status close() override {
        if (m_handle.dir == nullptr) {
            return Status::NOT_OPENED;
        }
        
        if (closedir(m_handle.dir) != 0) {
            return Status::OTHER_ERROR;
        }
        
        m_handle.dir = nullptr;
        return Status::OP_OK;
    }

    //! Get directory handle
    DirectoryHandle* getHandle() override {
        return &m_handle;
    }

  private:
    RtemsDirectoryHandle m_handle;
};

} // namespace Directory
} // namespace RTEMS
} // namespace Os

namespace Os {
DirectoryInterface* DirectoryInterface::getDelegate(DirectoryHandleStorage& aligned_new_memory) {
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::Directory::RtemsDirectory) <= sizeof(DirectoryHandleStorage),
                  "RTEMS directory implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Directory::RtemsDirectory)) == 0,
                  "Bad alignment for RTEMS directory implementation");
    return new (aligned_new_memory) Os::RTEMS::Directory::RtemsDirectory;
}
}