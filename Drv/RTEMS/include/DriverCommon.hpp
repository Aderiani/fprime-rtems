#ifndef DRV_RTEMS_DRIVER_COMMON_HPP
#define DRV_RTEMS_DRIVER_COMMON_HPP

// Include C++ standard headers first
#include <cstdint>
// Include F' headers
#include <FpConfig.hpp>

namespace Drv {
namespace RTEMS {

// GAISLER vendor ID and device IDs for reference
namespace DeviceId {
constexpr U16 GRGPIO = 0x01A;   // GPIO core
constexpr U16 APBUART = 0x00C;  // UART core
constexpr U16 SPICTRL = 0x02D;  // SPI controller
constexpr U16 I2CMST = 0x028;   // I2C master
}  // namespace DeviceId

// Base addresses for GR740 devices (update based on your board's memory map)
namespace BaseAddress {
constexpr std::uintptr_t GPIO = 0x80000A00;   // Example base address for GRGPIO
constexpr std::uintptr_t UART0 = 0x80000100;  // Example base address for APBUART0
constexpr std::uintptr_t UART1 = 0x80000200;  // Example base address for APBUART1
constexpr std::uintptr_t SPI = 0x80000300;    // Example base address for SPICTRL
constexpr std::uintptr_t I2C = 0x80000400;    // Example base address for I2CMST
}  // namespace BaseAddress

}  // namespace RTEMS
}  // namespace Drv

#endif  // DRV_RTEMS_DRIVER_COMMON_HPP