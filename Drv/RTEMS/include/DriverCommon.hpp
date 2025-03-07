// ======================================================================
// \title  DriverCommon.hpp
// \author fprime-community
// \brief  Common definitions for RTEMS drivers
//
// \copyright
// Copyright (C) 2024 fprime-community
// ALL RIGHTS RESERVED.
//
// ======================================================================

#ifndef DRV_RTEMS_DRIVER_COMMON_HPP
#define DRV_RTEMS_DRIVER_COMMON_HPP

#include <FpConfig.hpp>

// RTEMS includes
#include <rtems.h>
#include <bsp.h>
#include <rtems/libio.h>

// RTEMS Driver Manager includes
#include <drvmgr/drvmgr.h>
#include <drvmgr/drvmgr_confdefs.h>

// GRLIB RTEMS driver includes
#include <grlib/ambapp_bus.h>

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
     * Initializes the driver manager - this should be called once at startup
     * if not already initialized by the BSP
     */
    static void initializeDriverManager() {
        // If RTEMS_DRVMGR_STARTUP is defined, the BSP has already initialized
        // the driver manager, so we don't need to do it
#ifndef RTEMS_DRVMGR_STARTUP
        // Register the GRLIB AMBA Plug & Play bus as the root bus driver
        ambapp_grlib_root_register();
        
        // Initialize the driver manager
        drvmgr_init();
#endif
    }
    
    /**
     * Finds an AMBA device by device ID and instance
     * 
     * @param device_id Device ID to search for
     * @param instance Instance number (0-based)
     * @return Pointer to the device, or nullptr if not found
     */
    static struct drvmgr_dev* findAmbaDevice(U16 vendor_id, U16 device_id, int instance) {
        int index = 0;
        struct drvmgr_dev* dev = NULL;
        
        // Traverse all AMBA devices
        while ((dev = drvmgr_for_each_dev(&ambapp_bus_drv, dev)) != NULL) {
            // Get device information
            struct ambapp_dev *ambapp_dev = (struct ambapp_dev *)dev->businfo;
            
            // Check if device matches
            if ((ambapp_dev->id.vendor == vendor_id) && 
                (ambapp_dev->id.device == device_id)) {
                
                // If this is the instance we're looking for, return it
                if (index == instance) {
                    return dev;
                }
                index++;
            }
        }
        
        // Device not found
        return nullptr;
    }
};

} // namespace RTEMS
} // namespace Drv

#endif // DRV_RTEMS_DRIVER_COMMON_HPP