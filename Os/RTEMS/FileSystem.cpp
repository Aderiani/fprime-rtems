// ======================================================================
// \title Os/RTEMS/FileSystem.cpp
// \brief RTEMS implementation for Os::FileSystem (non-POSIX)
// ======================================================================
#include <Os/FileSystem.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>

#include "Os/RTEMS/FileSystem.hpp"

#include <rtems.h>
#include <rtems/libio.h>  // For rtems_filesystem_* functions
#include <errno.h>
#include <string.h>

namespace Os {
namespace RTEMS {
namespace FileSystem {

Os::FileSystemHandle RtemsFileSystem::s_handle;

RtemsFileSystem::RtemsFileSystem() {}

RtemsFileSystem::~RtemsFileSystem() {}

// Helper function to initialize a location info struct (minimal setup)
static void init_location(rtems_filesystem_location_info_t& loc) {
    loc.node_access = nullptr;
    loc.node_access_2 = nullptr;
    loc.handlers = nullptr;
    loc.mt_entry = nullptr;
    // Note: This is a simplification; proper setup requires filesystem context
}

// ----------------------------------------------------------------------
// _removeDirectory: Remove a directory at the specified path
// ----------------------------------------------------------------------
Os::FileSystemInterface::Status RtemsFileSystem::_removeDirectory(const char* path) {
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


// ----------------------------------------------------------------------
// _removeFile: Remove a file at the specified path
// ----------------------------------------------------------------------
Os::FileSystemInterface::Status RtemsFileSystem::_removeFile(const char* path) {
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

// ----------------------------------------------------------------------
// _rename: Rename/move a file or directory from sourcePath to destPath
// ----------------------------------------------------------------------
Os::FileSystemInterface::Status RtemsFileSystem::_rename(const char* sourcePath, const char* destPath) {
    FW_ASSERT(sourcePath != nullptr);
    FW_ASSERT(destPath != nullptr);

    rtems_filesystem_location_info_t old_loc;
    rtems_filesystem_location_info_t new_loc;
    rtems_filesystem_location_info_t parent_loc;
    init_location(old_loc);
    init_location(new_loc);
    init_location(parent_loc);

    // Use rtems_filesystem_default_rename
    int result = rtems_filesystem_default_rename(&old_loc, &new_loc, &parent_loc, sourcePath, strlen(sourcePath));
    if (result != 0) {
        switch (errno) {
            case ENOENT:
                return Os::FileSystemInterface::Status::DOESNT_EXIST;
            case EEXIST:
                return Os::FileSystemInterface::Status::ALREADY_EXISTS;
            case ENOSPC:
                return Os::FileSystemInterface::Status::NO_SPACE;
            case EACCES:
                return Os::FileSystemInterface::Status::NO_PERMISSION;
            case EXDEV:
                return Os::FileSystemInterface::Status::EXDEV_ERROR;
            default:
                return Os::FileSystemInterface::Status::OTHER_ERROR;
        }
    }

    return Os::FileSystemInterface::Status::OP_OK;
}

// ----------------------------------------------------------------------
// _getFreeSpace: Get total and free space for the filesystem at path
// ----------------------------------------------------------------------
Os::FileSystemInterface::Status RtemsFileSystem::_getFreeSpace(const char* path, FwSizeType& totalBytes, FwSizeType& freeBytes) {
    FW_ASSERT(path != nullptr);

    // No direct equivalent without POSIX statvfs; return NOT_SUPPORTED
    totalBytes = 0;
    freeBytes = 0;
    return Os::FileSystemInterface::Status::NOT_SUPPORTED;
}

// ----------------------------------------------------------------------
// _getWorkingDirectory: Get the current working directory
// ----------------------------------------------------------------------
Os::FileSystemInterface::Status RtemsFileSystem::_getWorkingDirectory(char* path, FwSizeType bufferSize) {
    FW_ASSERT(path != nullptr);
    FW_ASSERT(bufferSize > 0);

    // No direct equivalent without POSIX getcwd; return NOT_SUPPORTED
    return Os::FileSystemInterface::Status::NOT_SUPPORTED;
}

// ----------------------------------------------------------------------
// _changeWorkingDirectory: Change the current working directory
// ----------------------------------------------------------------------
Os::FileSystemInterface::Status RtemsFileSystem::_changeWorkingDirectory(const char* path) {
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

// ----------------------------------------------------------------------
// getHandle: Return the handle for this filesystem implementation
// ----------------------------------------------------------------------
Os::FileSystemHandle* RtemsFileSystem::getHandle() {
    return &s_handle;
}

} // namespace FileSystem
} // namespace RTEMS
} // namespace Os
