// ======================================================================
// \title Os/RTEMS/Console.cpp
// \brief RTEMS implementation for Os::Console
// ======================================================================
#include <Os/Console.hpp>
#include <Os/RTEMS/Console.hpp>
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <stdio.h>
#include <unistd.h>

namespace Os {
namespace RTEMS {
namespace Console {

struct RtemsConsoleHandle : public ConsoleHandle {
};

class RtemsConsole : public ConsoleInterface {
  public:
    //! Constructor
    RtemsConsole() = default;

    //! Destructor
    ~RtemsConsole() override = default;

    //! Implementation of required writeMessage function
    void writeMessage(const CHAR* message, const FwSizeType size) override {
        ::write(STDOUT_FILENO, message, size);
    }

    //! Get console handle
    ConsoleHandle* getHandle() override {
        return &m_handle;
    }

  private:
    RtemsConsoleHandle m_handle;
};

} // namespace Console
} // namespace RTEMS
} // namespace Os

namespace Os {
ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory, const ConsoleInterface* to_copy) {
    // Don't assert on a reference parameter
    static_assert(sizeof(Os::RTEMS::Console::RtemsConsole) <= sizeof(ConsoleHandleStorage),
                  "RTEMS console implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Console::RtemsConsole)) == 0,
                  "Bad alignment for RTEMS console handle");
    return new (aligned_new_memory) Os::RTEMS::Console::RtemsConsole();
}
}