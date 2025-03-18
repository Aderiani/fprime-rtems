#ifndef DRV_RTEMS_DRIVER_COMMON_HPP
#define DRV_RTEMS_DRIVER_COMMON_HPP

#include <FpConfig.hpp>

// RTEMS includes
#include <rtems.h>
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp.h>

// Suppress pedantic warnings for system headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <drvmgr/drvmgr.h>
#include <grlib/ambapp.h>
#pragma GCC diagnostic pop

namespace Drv {
namespace RTEMS {

// GAISLER vendor ID
constexpr U16 GAISLER_VENDOR_ID = 0x01;

// GAISLER device IDs that we use in our drivers
namespace DeviceId {
    constexpr U16 GRGPIO   = 0x01A; // GPIO core
    constexpr U16 APBUART  = 0x00C; // UART core
    constexpr U16 SPICTRL  = 0x02D; // SPI controller
    constexpr U16 I2CMST   = 0x028; // I2C master
}

// Common utility functions for RTEMS drivers
class DriverUtil {
public:
    /**
     * Finds an AMBA device by device ID and instance
     * 
     * @param vendor_id Vendor ID to search for
     * @param device_id Device ID to search for
     * @param instance Instance number (0-based)
     * @return Pointer to the device, or nullptr if not found
     */
    static struct drvmgr_dev* findAmbaDevice(U16 vendor_id, U16 device_id, int instance) {
        int index = 0;
        struct drvmgr_dev* dev = NULL;
        
        // Get the AMBA bus
        struct drvmgr_bus* abus = drvmgr_get_bus(&ambapp_bus_drv, 0);
        if (!abus)
            return NULL;
        
        // Iterate through devices on the bus
        while ((dev = drvmgr_get_dev(abus, DRVMGR_BUS_DEVICE, index)) != NULL) {
            if (dev->businfo) {
                struct ambapp_dev* ambapp_dev = (struct ambapp_dev*)dev->businfo;
                
                // Check if this device matches the vendor and device ID
                if ((ambapp_dev->id.vendor == vendor_id) && 
                    (ambapp_dev->id.device == device_id)) {
                    
                    // If this is the instance we're looking for, return it
                    if (instance == 0) {
                        return dev;
                    }
                    instance--;
                }
            }
            index++;
        }
        
        // Device not found
        return NULL;
    }
        
        // Device not found
        return NULL;
    }
};

} // namespace RTEMS
} // namespace Drv

#endif // DRV_RTEMS_DRIVER_COMMON_HPP