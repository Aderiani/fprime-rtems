#ifndef OS_RTEMS_FILESYSTEM_HPP
#define OS_RTEMS_FILESYSTEM_HPP

#include <Os/FileSystem.hpp>

namespace Os {
namespace RTEMS {
namespace FileSystem {

class RtemsFileSystem : public Os::FileSystemInterface {
public:
    RtemsFileSystem();
    virtual ~RtemsFileSystem();

    Os::FileSystemInterface::Status _removeDirectory(const char* path) override;
    Os::FileSystemInterface::Status _removeFile(const char* path) override;
    Os::FileSystemInterface::Status _rename(const char* sourcePath, const char* destPath) override;
    Os::FileSystemInterface::Status _getFreeSpace(const char* path, FwSizeType& totalBytes, FwSizeType& freeBytes) override;
    Os::FileSystemInterface::Status _getWorkingDirectory(char* path, FwSizeType bufferSize) override;
    Os::FileSystemInterface::Status _changeWorkingDirectory(const char* path) override;

    Os::FileSystemHandle* getHandle() override;

private:
    static Os::FileSystemHandle s_handle;
};

} // namespace FileSystem
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_FILESYSTEM_HPP