// ======================================================================
// \title Os/RTEMS/DefaultConsole.cpp
// \brief sets default Os::Console to RTEMS implementation via linker
// ======================================================================
#include "Os/Console.hpp"
#include "Os/RTEMS/Console.hpp"
#include "Os/Delegate.hpp"
#include <stdio.h>
#include <string.h> // For memset

namespace Os {
ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory, const ConsoleInterface* to_copy) {
    // Ignore to_copy parameter if provided
    return Os::Delegate::makeDelegate<ConsoleInterface, Os::RTEMS::Console::RtemsConsole>(aligned_new_memory);
}

}