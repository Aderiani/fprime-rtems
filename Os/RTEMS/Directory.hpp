// ======================================================================
// \title Os/RTEMS/Directory.hpp
// \brief RTEMS implementation for Os::Directory, header file
// ======================================================================
#ifndef OS_RTEMS_DIRECTORY_HPP
#define OS_RTEMS_DIRECTORY_HPP

#include <Os/Directory.hpp>
#include <dirent.h>

namespace Os {
namespace RTEMS {
namespace Directory {

//! DirectoryHandle class definition for RTEMS implementations
struct RtemsDirectoryHandle : public DirectoryHandle {
    DIR* dir;  // Directory stream pointer
    RtemsDirectoryHandle() : dir(nullptr) {}
};

//! \brief RTEMS implementation of Os::DirectoryInterface
class RtemsDirectory : public DirectoryInterface {
  public:
    //! Constructor
    RtemsDirectory();

    //! Destructor
    ~RtemsDirectory() override;

    // Override required pure virtual methods from DirectoryInterface
    Status open(const char* path, OpenMode mode) override;
    Status read(char* buffer, PlatformSizeType maxSize) override;
    Status rewind() override;
    void close() override;
    DirectoryHandle* getHandle() override;

  private:
    RtemsDirectoryHandle m_handle;
};

} // namespace Directory
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_DIRECTORY_HPP