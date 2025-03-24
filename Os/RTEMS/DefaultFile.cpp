// ======================================================================
// \title Os/RTEMS/DefaultFile.cpp
// \brief sets default Os::File to RTEMS implementation via linker
// ======================================================================
#include "Os/File.hpp"
#include "Os/RTEMS/File.hpp"
#include "Os/Delegate.hpp"

namespace Os {
FileInterface* FileInterface::getDelegate(FileHandleStorage& aligned_new_memory, const FileInterface* to_copy) {
    return Os::Delegate::makeDelegate<FileInterface, Os::RTEMS::File::RtemsFile>(aligned_new_memory);
}
}