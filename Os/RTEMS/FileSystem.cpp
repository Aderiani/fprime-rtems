// ======================================================================
// \title Os/RTEMS/FileSystem.cpp
// \brief RTEMS implementation for Os::FileSystem
// ======================================================================
#include <Os/FileSystem.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include "Os/RTEMS/FileSystem.hpp"

#include <rtems.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/statvfs.h>
#include <libgen.h>

namespace Os {
namespace RTEMS {
namespace FileSystem {

RtemsFileSystemHandle RtemsFileSystem::s_handle;

RtemsFileSystem::RtemsFileSystem() {
}

RtemsFileSystem::~RtemsFileSystem() {
}

Status RtemsFileSystem::_createDirectory(const char* path) {
    FW_ASSERT(path != nullptr);
    
    if (mkdir(path, 0755) != 0) {
        switch (errno) {
            case EEXIST:
                return Status::ALREADY_EXISTS;
            case ENOSPC:
                return Status::NO_SPACE;
            case EACCES:
                return Status::NO_PERMISSION;
            case ENOTDIR:
                return Status::NOT_DIR;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_removeDirectory(const char* path) {
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
                return Status::NOT_EMPTY;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_removeFile(const char* path) {
    FW_ASSERT(path != nullptr);
    
    if (unlink(path) != 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            case EISDIR:
                return Status::IS_DIR;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_rename(const char* sourcePath, const char* destPath) {
    FW_ASSERT(sourcePath != nullptr);
    FW_ASSERT(destPath != nullptr);
    
    if (rename(sourcePath, destPath) != 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            case EISDIR:
                return Status::IS_DIR;
            case EXDEV:
                return Status::EXDEV_ERROR;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_moveFile(const char* sourcePath, const char* destPath) {
    // In POSIX, rename can be used for moving files
    return _rename(sourcePath, destPath);
}

Status RtemsFileSystem::_copyFile(const char* sourcePath, const char* destPath) {
    FW_ASSERT(sourcePath != nullptr);
    FW_ASSERT(destPath != nullptr);
    
    // Open source file
    int src_fd = open(sourcePath, O_RDONLY);
    if (src_fd < 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    // Open destination file
    int dst_fd = open(destPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) {
        close(src_fd);
        switch (errno) {
            case ENOSPC:
                return Status::NO_SPACE;
            case EACCES:
                return Status::NO_PERMISSION;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    // Copy data
    char buffer[FW_FILE_CHUNK_SIZE];
    ssize_t n;
    
    while ((n = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dst_fd, buffer, n) != n) {
            close(src_fd);
            close(dst_fd);
            return Status::OTHER_ERROR;
        }
    }
    
    close(src_fd);
    close(dst_fd);
    
    if (n < 0) {
        return Status::OTHER_ERROR;
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_getFileSize(const char* path, FwSizeType& size) {
    FW_ASSERT(path != nullptr);
    
    struct stat st;
    if (stat(path, &st) != 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    size = static_cast<FwSizeType>(st.st_size);
    return Status::OP_OK;
}

Status RtemsFileSystem::_changeWorkingDirectory(const char* path) {
    FW_ASSERT(path != nullptr);
    
    if (chdir(path) != 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            case ENOTDIR:
                return Status::NOT_DIR;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_getWorkingDirectory(char* path, FwSizeType bufferSize) {
    FW_ASSERT(path != nullptr);
    
    if (getcwd(path, bufferSize) == nullptr) {
        switch (errno) {
            case ERANGE:
                return Status::BUFFER_TOO_SMALL;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_fileExists(const char* path, bool& exists) {
    FW_ASSERT(path != nullptr);
    
    struct stat st;
    if (stat(path, &st) == 0) {
        exists = true;
    } else {
        exists = false;
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_getFileStats(const char* path, const FileSystem::Stats* stats) {
    FW_ASSERT(path != nullptr);
    FW_ASSERT(stats != nullptr);
    
    struct stat st;
    if (stat(path, &st) != 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    // Fill in stats struct
    FileSystem::Stats& s = const_cast<FileSystem::Stats&>(*stats);
    s.size = static_cast<FwSizeType>(st.st_size);
    s.isDirectory = S_ISDIR(st.st_mode);
    s.permissions = st.st_mode & 0777;
    
    // Extract time information
    s.lastModified.seconds = static_cast<U32>(st.st_mtime);
    s.lastModified.microseconds = 0; // RTEMS stat doesn't provide microseconds
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_appendPath(const char* basePath, const char* appendPath, char* destPath, FwSizeType& destPathSize) {
    FW_ASSERT(basePath != nullptr);
    FW_ASSERT(appendPath != nullptr);
    FW_ASSERT(destPath != nullptr);
    
    // Check if the path would be too long
    size_t baseLen = strlen(basePath);
    size_t appendLen = strlen(appendPath);
    size_t needLen = baseLen + 1 + appendLen + 1; // +1 for separator, +1 for null terminator
    
    if (needLen > destPathSize) {
        return Status::BUFFER_TOO_SMALL;
    }
    
    // Copy base path
    strncpy(destPath, basePath, destPathSize);
    
    // Add separator if needed
    if (baseLen > 0 && basePath[baseLen - 1] != '/') {
        destPath[baseLen] = '/';
        strncpy(destPath + baseLen + 1, appendPath, destPathSize - baseLen - 1);
    } else {
        strncpy(destPath + baseLen, appendPath, destPathSize - baseLen);
    }
    
    destPathSize = needLen - 1; // Don't count null terminator
    return Status::OP_OK;
}

Status RtemsFileSystem::_getFreeSpace(const char* path, FwSizeType& totalBytes, FwSizeType& freeBytes) {
    FW_ASSERT(path != nullptr);
    
    struct statvfs st;
    if (statvfs(path, &st) != 0) {
        switch (errno) {
            case ENOENT:
                return Status::DOESNT_EXIST;
            case EACCES:
                return Status::NO_PERMISSION;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    // Calculate free and total space
    freeBytes = static_cast<FwSizeType>(st.f_bavail * st.f_frsize);
    totalBytes = static_cast<FwSizeType>(st.f_blocks * st.f_frsize);
    
    return Status::OP_OK;
}

Status RtemsFileSystem::_getPlatformID(PlatformID& id) {
    // RTEMS specific platform ID
    id = PlatformID::RTEMS;
    return Status::OP_OK;
}

FileSystemHandle* RtemsFileSystem::getHandle() {
    return &s_handle;
}

} // namespace FileSystem
} // namespace RTEMS
} // namespace Os

namespace Os {
FileSystemInterface* FileSystemInterface::getDelegate(FileSystemHandleStorage& aligned_new_memory) {
    // References cannot be null, so we can safely remove this assertion
    static_assert(sizeof(Os::RTEMS::FileSystem::RtemsFileSystem) <= sizeof(FileSystemHandleStorage),
                  "RTEMS FileSystem implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::FileSystem::RtemsFileSystem)) == 0,
                  "Bad alignment for RTEMS FileSystem implementation");
    return new (aligned_new_memory) Os::RTEMS::FileSystem::RtemsFileSystem();
}
}