// ======================================================================
// \title Os/RTEMS/Console.cpp
// \brief RTEMS implementation for Os::Console
// ======================================================================
#include <Os/Console.hpp>
#include <Os/Delegate.hpp>
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

    //! Write to the console
    Status write(const char* buffer, PlatformSizeType& size) override {
        const ssize_t result = ::write(STDOUT_FILENO, buffer, size);
        if (result >= 0) {
            size = static_cast<PlatformSizeType>(result);
            return Status::OP_OK;
        }
        return Status::ERROR;
    }

    //! Write to the console
    Status flush() override {
        ::fflush(stdout);
        return Status::OP_OK;
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
ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory) {
    FW_ASSERT(aligned_new_memory != nullptr);
    static_assert(sizeof(Os::RTEMS::Console::RtemsConsole) <= sizeof(ConsoleHandleStorage),
                  "RTEMS console implementation too large");
    static_assert((FW_HANDLE_ALIGNMENT % alignof(Os::RTEMS::Console::RtemsConsole)) == 0,
                  "Bad alignment for RTEMS console handle");
    return new (aligned_new_memory) Os::RTEMS::Console::RtemsConsole;
}
}