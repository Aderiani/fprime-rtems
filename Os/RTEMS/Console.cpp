// ======================================================================
// \title Os/RTEMS/Console.cpp
// \brief RTEMS implementation for Os::Console
// ======================================================================
#include "Os/RTEMS/Console.hpp"
#include <unistd.h>  // For write() and STDOUT_FILENO
#include <stdio.h>

namespace Os {
namespace RTEMS {
namespace Console {

RtemsConsole::RtemsConsole() {
    // Default constructor implementation (if needed)
}

RtemsConsole::~RtemsConsole() {
    // Default destructor implementation (if needed)
}


void RtemsConsole::writeMessage(const CHAR* message, const FwSizeType size) {
    // Add a debug print
    printf("Console writing: %.*s\n", (int)size, message);
    ::write(STDOUT_FILENO, message, size);
}

ConsoleHandle* RtemsConsole::getHandle() {
    return &m_handle;
}

} // namespace Console
} // namespace RTEMS
} // namespace Os