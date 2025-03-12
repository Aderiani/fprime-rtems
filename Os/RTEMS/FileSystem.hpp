// ======================================================================
// \title Os/RTEMS/FileSystem.hpp
// \brief RTEMS implementation for Os::FileSystem
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
    RtemsFileSystem() = default;
    ~RtemsFileSystem() override = default;

    Status createDirectory(const char* path) override;
    Status removeDirectory(const char* path) override;
    Status removeFile(const char* path) override;
    Status moveFile(const char* file1, const char* file2) override;
    Status copyFile(const char* file1, const char* file2) override;
    Status getFileSize(const char* path, PlatformSizeType& size) override;
    Status changeWorkingDirectory(const char* path) override;
    Status getWorkingDirectory(char* path, PlatformSizeType pathSize) override;
    Status exists(const char* path, bool& doesExist) override;
    Status getInfo(const char* path, Info* info) override;
    Status appendPath(const char* basePath, const char* subsPath, char* fullPath, PlatformSizeType fullPathSize) override;
    Status getFileCount(const char* path, PlatformSizeType& fileCount) override;
    Status getFreeSpace(const char* path, PlatformSizeType& freeSpace, PlatformSizeType& totalSpace) override;

    FileSystemHandle* getHandle() override;

  private:
    static RtemsFileSystemHandle s_handle;
};

} // namespace FileSystem
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_FILESYSTEM_HPP