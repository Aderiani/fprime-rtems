// ======================================================================
// \title Os/RTEMS/Cpu.cpp
// \brief RTEMS implementation for Os::Cpu with SMP support
// ======================================================================
#include "Os/RTEMS/Cpu.hpp"
#include <Os/Delegate.hpp>
#include <Fw/Types/Assert.hpp>
#include <rtems.h>
#include <rtems/cpuuse.h>
#include <rtems/score/percpu.h>
#include <rtems/score/timestamp.h>
#include <cstdio>

namespace Os {
namespace RTEMS {
namespace Cpu {

typedef struct {
    Timestamp_Control prev_total;
    Timestamp_Control prev_idle;
    bool initialized;
} CpuTracker;

static CpuTracker s_trackers[CONFIGURE_MAXIMUM_PROCESSORS] = {};

CpuInterface::Status RtemsCpu::_getCount(FwSizeType& cpu_count) {
#ifdef RTEMS_SMP
    cpu_count = rtems_scheduler_get_processor_maximum();
#else
    cpu_count = 1;
#endif
    return CpuInterface::Status::OP_OK;
}

CpuInterface::Status RtemsCpu::_getTicks(Os::Cpu::Ticks& ticks, FwSizeType cpu_index) {
    FwSizeType count = 0;
    CpuInterface::Status status = _getCount(count);
    
    if (status != CpuInterface::Status::OP_OK || cpu_index >= count) {
        return CpuInterface::Status::ERROR;
    }

    // Get CPU usage statistics
    Timestamp_Control current_total, current_idle;
    rtems_cpu_usage_get(cpu_index, &current_total, &current_idle);

    CpuTracker& tracker = s_trackers[cpu_index];

    if (!tracker.initialized) {
        tracker.prev_total = current_total;
        tracker.prev_idle = current_idle;
        tracker.initialized = true;
        ticks.total = 100;
        ticks.used = 0;
        return CpuInterface::Status::OP_OK;
    }

    // Calculate differences
    Timestamp_Control delta_total = current_total - tracker.prev_total;
    Timestamp_Control delta_idle = current_idle - tracker.prev_idle;

    // Convert to nanoseconds for precision
    uint64_t delta_total_ns = _Timestamp_Get_nanoseconds(&delta_total);
    uint64_t delta_idle_ns = _Timestamp_Get_nanoseconds(&delta_idle);

    // Avoid division by zero
    if (delta_total_ns == 0) {
        ticks.total = 1;
        ticks.used = 0;
        return CpuInterface::Status::OP_OK;
    }

    // Calculate usage: (total - idle) / total * 100
    ticks.total = delta_total_ns;
    ticks.used = delta_total_ns - delta_idle_ns;

    // Update tracker
    tracker.prev_total = current_total;
    tracker.prev_idle = current_idle;

    return CpuInterface::Status::OP_OK;
}

CpuHandle* RtemsCpu::getHandle() {
    return &m_handle;
}

} // namespace Cpu
} // namespace RTEMS
} // namespace Os