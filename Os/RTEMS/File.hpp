// ======================================================================
// \title Os/RTEMS/File.hpp
// \brief RTEMS implementation for Os::File, header file
// ======================================================================
#ifndef OS_RTEMS_FILE_HPP
#define OS_RTEMS_FILE_HPP

#include <Os/File.hpp>

namespace Os {
namespace RTEMS {
namespace File {

//! FileHandle class definition for RTEMS implementations
struct RtemsFileHandle : public FileHandle {
    int fd; // File descriptor
    RtemsFileHandle() : fd(-1) {}
};

//! \brief RTEMS implementation of Os::FileInterface
class RtemsFile : public FileInterface {
  public:
    //! Constructor
    RtemsFile();

    //! Destructor
    ~RtemsFile() override;

    Status open(const char* path, Mode mode, OverwriteType overwrite = OverwriteType::OVERWRITE) override;
    Status read(U8* buffer, FwSignedSizeType& size, WaitType wait = WaitType::WAIT) override;
    Status write(const U8* buffer, FwSignedSizeType& size, WaitType wait = WaitType::WAIT) override;
    Status seek(FwSignedSizeType offset, SeekType seekType) override;
    Status size(FwSignedSizeType& size_result) override;
    Status position(FwSignedSizeType& position_result) override;
    Status preallocate(FwSignedSizeType offset, FwSignedSizeType length) override;
    Status flush() override;
    void close() override;
    FileHandle* getHandle() override;
    
    // Custom method - not overriding anything from parent
    Status readline(U8* buffer, FwSignedSizeType& size, WaitType wait = WaitType::WAIT);
    
  private:
    RtemsFileHandle m_handle;
};

} // namespace File
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_FILE_HPP