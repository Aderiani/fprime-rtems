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
// ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory, const ConsoleInterface* to_copy) {
//     // Ignore to_copy parameter if provided
//     return Os::Delegate::makeDelegate<ConsoleInterface, Os::RTEMS::Console::RtemsConsole>(aligned_new_memory);
// }

// In Os/RTEMS/Console.cpp
ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory, const ConsoleInterface* to_copy) {
    printf("Creating Console delegate - start\n");
    fflush(stdout);
    
    // Manually ensure the memory is properly aligned and initialized
    memset(&aligned_new_memory, 0, sizeof(ConsoleHandleStorage));
    
    // Create the console object directly without using Delegate::makeDelegate
    Os::RTEMS::Console::RtemsConsole* console = 
        new (&aligned_new_memory) Os::RTEMS::Console::RtemsConsole();
    
    printf("Console delegate created - end\n");
    fflush(stdout);
    
    return console;
}

}