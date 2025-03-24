// ======================================================================
// \title Os/RTEMS/Console.hpp
// \brief RTEMS implementation for Os::Console, header file
// ======================================================================
#ifndef OS_RTEMS_CONSOLE_HPP
#define OS_RTEMS_CONSOLE_HPP

#include <Os/Console.hpp>

namespace Os {
namespace RTEMS {
namespace Console {

//! ConsoleHandle class definition for RTEMS implementations
struct RtemsConsoleHandle : public ConsoleHandle {
    // No additional members needed for RTEMS console
};

//! \brief RTEMS implementation of Os::ConsoleInterface
class RtemsConsole : public ConsoleInterface {
  public:
    //! Constructor
    RtemsConsole();

    //! Destructor
    ~RtemsConsole() override;

    //! Implementation of required writeMessage function
    void writeMessage(const CHAR* message, const FwSizeType size) override;

    //! Get console handle
    ConsoleHandle* getHandle() override;

  private:
    RtemsConsoleHandle m_handle;
};

} // namespace Console
} // namespace RTEMS
} // namespace Os

#endif // OS_RTEMS_CONSOLE_HPP