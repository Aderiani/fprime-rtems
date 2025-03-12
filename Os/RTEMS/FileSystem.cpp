// ======================================================================
// \title Os/RTEMS/FileSystem.cpp
// \brief RTEMS implementation for Os::FileSystem
// ======================================================================
#include <Os/RTEMS/FileSystem.hpp>
#include <Fw/Types/Assert.hpp>

#include <rtems.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>

namespace Os {
namespace RTEMS {
namespace FileSystem {

RtemsFileSystemHandle RtemsFileSystem::s_handle;

Status RtemsFileSystem::createDirectory(const char* path) {
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

Status RtemsFileSystem::removeDirectory(const char* path) {
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

Status RtemsFileSystem::removeFile(const char* path) {
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

Status RtemsFileSystem::moveFile(const char* file1, const char* file2) {
    FW_ASSERT(file1 != nullptr);
    FW_ASSERT(file2 != nullptr);
    
    if (rename(file1, file2) != 0) {
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

Status RtemsFileSystem::copyFile(const char* file1, const char* file2) {
    FW_ASSERT(file1 != nullptr);
    FW_ASSERT(file2 != nullptr);
    
    // Open source file
    int src_fd = open(file1, O_RDONLY);
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
    int dst_fd = open(file2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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

Status RtemsFileSystem::getFileSize(const char* path, PlatformSizeType& size) {
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
    
    size = static_cast<PlatformSizeType>(st.st_size);
    return Status::OP_OK;
}

Status RtemsFileSystem::changeWorkingDirectory(const char* path) {
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

Status RtemsFileSystem::getWorkingDirectory(char* path, PlatformSizeType pathSize) {
    FW_ASSERT(path != nullptr);
    
    if (getcwd(path, pathSize) == nullptr) {
        switch (errno) {
            case ERANGE:
                return Status::BUFFER_TOO_SMALL;
            default:
                return Status::OTHER_ERROR;
        }
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::exists(const char* path, bool& doesExist) {
    FW_ASSERT(path != nullptr);
    
    struct stat st;
    if (stat(path, &st) == 0) {
        doesExist = true;
    } else {
        doesExist = false;
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::getInfo(const char* path, Info* info) {
    FW_ASSERT(path != nullptr);
    FW_ASSERT(info != nullptr);
    
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
    
    // Set info fields
    info->size = static_cast<PlatformSizeType>(st.st_size);
    info->isDir = S_ISDIR(st.st_mode);
    info->permissions = st.st_mode & 0777;
    
    // Extract time information
    info->lastModTime.seconds = static_cast<U32>(st.st_mtime);
    info->lastModTime.microseconds = 0; // RTEMS stat doesn't provide microseconds
    
    return Status::OP_OK;
}

Status RtemsFileSystem::appendPath(const char* basePath, const char* subsPath, char* fullPath, PlatformSizeType fullPathSize) {
    FW_ASSERT(basePath != nullptr);
    FW_ASSERT(subsPath != nullptr);
    FW_ASSERT(fullPath != nullptr);
    
    // Check if the path would be too long
    size_t baseLen = strlen(basePath);
    size_t subsLen = strlen(subsPath);
    size_t needLen = baseLen + 1 + subsLen + 1; // +1 for separator, +1 for null terminator
    
    if (needLen > fullPathSize) {
        return Status::BUFFER_TOO_SMALL;
    }
    
    // Copy base path
    strncpy(fullPath, basePath, fullPathSize);
    
    // Add separator if needed
    if (baseLen > 0 && basePath[baseLen - 1] != '/') {
        fullPath[baseLen] = '/';
        strncpy(fullPath + baseLen + 1, subsPath, fullPathSize - baseLen - 1);
    } else {
        strncpy(fullPath + baseLen, subsPath, fullPathSize - baseLen);
    }
    
    return Status::OP_OK;
}

Status RtemsFileSystem::getFileCount(const char* path, PlatformSizeType& fileCount) {
    FW_ASSERT(path != nullptr);
    
    DIR* dir = opendir(path);
    if (dir == nullptr) {
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
    
    fileCount = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            fileCount++;
        }
    }
    
    closedir(dir);
    return Status::OP_OK;
}

Status RtemsFileSystem::getFreeSpace(const char* path, PlatformSizeType& freeSpace, PlatformSizeType& totalSpace) {
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
    freeSpace = static_cast<PlatformSizeType>(st.f_bavail * st.f_frsize);
    totalSpace = static_cast<PlatformSizeType>(st.f_blocks * st.f_frsize);
    
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
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::FileSystem::RtemsFileSystem) <= sizeof(FileSystemHandleStorage),
                  "RTEMS FileSystem implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::FileSystem::RtemsFileSystem)) == 0,
                  "Bad alignment for RTEMS FileSystem implementation");
    return new (aligned_new_memory) Os::RTEMS::FileSystem::RtemsFileSystem;
}
}