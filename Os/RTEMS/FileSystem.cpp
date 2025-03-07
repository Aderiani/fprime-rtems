#include <Os/FileSystem.hpp>
#include <rtems.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

namespace Os {

    FileSystem::Status FileSystem::createDirectory(const Fw::StringBase& path) {
        if (mkdir(path.toChar(), 0777) < 0) {
            return handleFileError(errno);
        }
        
        return OP_OK;
    }

    FileSystem::Status FileSystem::removeDirectory(const Fw::StringBase& path) {
        if (rmdir(path.toChar()) < 0) {
            return handleFileError(errno);
        }
        
        return OP_OK;
    }

    FileSystem::Status FileSystem::removeFile(const Fw::StringBase& path) {
        if (unlink(path.toChar()) < 0) {
            return handleFileError(errno);
        }
        
        return OP_OK;
    }

    FileSystem::Status FileSystem::moveFile(const Fw::StringBase& source, const Fw::StringBase& target) {
        if (rename(source.toChar(), target.toChar()) < 0) {
            return handleFileError(errno);
        }
        
        return OP_OK;
    }

    FileSystem::Status FileSystem::handleFileError(int errorNumber) {
        switch (errorNumber) {
            case ENOENT:
                return DOESNT_EXIST;
                
            case EACCES:
                return NO_PERMISSION;
                
            case ENOTDIR:
                return NOT_DIR;
                
            case EISDIR:
                return IS_DIR;
                
            case ENOTEMPTY:
                return NOT_EMPTY;
                
            case EINVAL:
                return INVALID_PATH;
                
            case EEXIST:
                return ALREADY_EXISTS;
                
            case EMFILE:
            case ENFILE:
                return FILE_LIMIT;
                
            case EBUSY:
                return BUSY;
                
            case ENOSPC:
                return NO_SPACE;
                
            case ENAMETOOLONG:
                return INVALID_PATH;
                
            case ENODATA:
                return NO_MORE_FILES;
                
            case ERANGE:
                return BUFFER_TOO_SMALL;
                
            case EXDEV:
                return EXDEV_ERROR;
                
            case EOVERFLOW:
                return OVERFLOW_ERROR;
                
            case ENOSYS:
                return NOT_SUPPORTED;
                
            default:
                return OTHER_ERROR;
        }
    }
}