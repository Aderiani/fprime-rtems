// ======================================================================
// \title Os/RTEMS/DefaultFileSystem.cpp
// \brief sets default Os::FileSystem to RTEMS implementation via linker
// ======================================================================
#include "Os/FileSystem.hpp"
#include "Os/RTEMS/FileSystem.hpp"
#include "Os/Delegate.hpp"

namespace Os {
FileSystemInterface* FileSystemInterface::getDelegate(FileSystemHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<FileSystemInterface, Os::RTEMS::FileSystem::RtemsFileSystem>(aligned_new_memory);
}
}