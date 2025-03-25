// ======================================================================
// \title Os/RTEMS/Directory.cpp
// \brief RTEMS implementation for Os::Directory
// ======================================================================
#include <Os/RTEMS/Directory.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

// System includes needed for RTEMS implementation
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

namespace Os {
namespace RTEMS {
namespace Directory {

RtemsDirectory::RtemsDirectory() {
    m_handle.dir = nullptr;
}

RtemsDirectory::~RtemsDirectory() {
    if (m_handle.dir != nullptr) {
        this->close();
    }
}

Os::DirectoryInterface::Status RtemsDirectory::open(const char* path, OpenMode mode) {
    FW_ASSERT(path != nullptr);
    
    if (m_handle.dir != nullptr) {
        return Status::OP_OK; // Already opened
    }

    struct stat st;
    if (stat(path, &st) == 0) {
        // Path exists, check if it's a directory
        if (!S_ISDIR(st.st_mode)) {
            return Status::NOT_DIR;
        }
        
        // For exclusive creation mode, return error if directory exists
        if (mode == OpenMode::CREATE_EXCLUSIVE) {
            return Status::ALREADY_EXISTS;
        }
        
        // Open existing directory
        m_handle.dir = opendir(path);
        if (m_handle.dir == nullptr) {
            if (errno == EACCES) {
                return Status::NO_PERMISSION;
            }
            return Status::OTHER_ERROR;
        }
        
        return Status::OP_OK;
    } else {
        // Path doesn't exist
        if (errno == ENOENT) {
            if (mode == OpenMode::READ) {
                return Status::DOESNT_EXIST;
            }
            
            // Try to create the directory if mode allows it
            if (mode == OpenMode::CREATE_IF_MISSING || mode == OpenMode::CREATE_EXCLUSIVE) {
                if (mkdir(path, 0755) != 0) {
                    if (errno == EACCES) {
                        return Status::NO_PERMISSION;
                    }
                    return Status::OTHER_ERROR;
                }
                
                // Open the newly created directory
                m_handle.dir = opendir(path);
                if (m_handle.dir == nullptr) {
                    return Status::OTHER_ERROR;
                }
                
                return Status::OP_OK;
            }
        }
        
        return Status::OTHER_ERROR;
    }
}

Os::DirectoryInterface::Status RtemsDirectory::read(char* buffer, PlatformSizeType maxSize) {
    FW_ASSERT(buffer != nullptr);
    
    if (m_handle.dir == nullptr) {
        return Status::NOT_OPENED;
    }
    
    errno = 0;
    struct dirent* entry = readdir(m_handle.dir);
    
    if (entry == nullptr) {
        if (errno != 0) {
            return Status::OTHER_ERROR;
        }
        return Status::NO_MORE_FILES;
    }
    
    PlatformSizeType nameLen = strlen(entry->d_name);
    if (nameLen >= maxSize) {
        return Status::FILE_LIMIT;
    }
    
    strncpy(buffer, entry->d_name, maxSize);
    buffer[maxSize - 1] = '\0'; // Ensure null termination
    
    return Status::OP_OK;
}

Os::DirectoryInterface::Status RtemsDirectory::rewind() {
    if (m_handle.dir == nullptr) {
        return Status::NOT_OPENED;
    }
    
    ::rewinddir(m_handle.dir);
    return Status::OP_OK;
}

void RtemsDirectory::close() {
    if (m_handle.dir != nullptr) {
        closedir(m_handle.dir);
        m_handle.dir = nullptr;
    }
}

DirectoryHandle* RtemsDirectory::getHandle() {
    return &m_handle;
}

} // namespace Directory
} // namespace RTEMS
} // namespace Os

