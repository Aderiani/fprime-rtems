// ======================================================================
// \title Os/RTEMS/FileSystem.hpp
// \brief RTEMS implementation for Os::FileSystem, header file
// ======================================================================
#ifndef OS_RTEMS_FILESYSTEM_HPP
#define OS_RTEMS_FILESYSTEM_HPP

#include <Os/FileSystem.hpp>

namespace Os {
namespace RTEMS {
namespace FileSystem {

struct RtemsFileSystemHandle : public FileSystemHandle {
};

class RtemsFileSystem : public FileSystemInterface {
  public:
    RtemsFileSystem();
    ~RtemsFileSystem() override;

    // Override required pure virtual methods from FileSystemInterface
    Status _createDirectory(const char* path) override;
    Status _removeDirectory(const char* path) override;
    Status _removeFile(const char* path) override;
    Status _rename(const char* sourcePath, const char* destPath) override;
    Status _moveFile(const char* sourcePath, const char* destPath) override;
    Status _copyFile(const char* sourcePath, const char* destPath) override;
    Status _getFreeSpace(const char* path, FwSizeType& totalBytes, FwSizeType& freeBytes) override;
    Status _getWorkingDirectory(char* path, FwSizeType bufferSize) override;
    Status _changeWorkingDirectory(const char* path) override;
    Status _fileExists(const char* path, bool& exists) override;
    Status _getFileSize(const char* path, FwSizeType& size) override;
    Status _getFileStats(const char* path, const FileSystem::Stats* stats) override;
    Status _appendPath(const char* basePath, const char* appendPath, char* destPath, FwSizeType& destPathSize) override;
    Status _getPlatformID(PlatformID& id) override;

    FileSystemHandle* getHandle() override;

  private:
    static RtemsFileSystemHandle s_handle;
};

} // namespace FileSystem
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_FILESYSTEM_HPP