// ======================================================================
// \title Os/RTEMS/DefaultDirectory.cpp
// \brief sets default Os::Directory to RTEMS implementation via linker
// ======================================================================
#include "Os/Directory.hpp"
#include "Os/RTEMS/Directory.hpp"
#include "Os/Delegate.hpp"

namespace Os {
DirectoryInterface* DirectoryInterface::getDelegate(DirectoryHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<DirectoryInterface, Os::RTEMS::Directory::RtemsDirectory>(aligned_new_memory);
}
}