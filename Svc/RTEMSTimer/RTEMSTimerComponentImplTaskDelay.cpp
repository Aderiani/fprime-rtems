// ======================================================================
// \title  RTEMSTimerImpl.cpp
// \author tim
// \brief  cpp file for RTEMSTimer component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================


#include <Svc/RTEMSTimer/RTEMSTimerComponentImpl.hpp>
#include <FpConfig.hpp>
#include <Os/Task.hpp>

namespace Svc {

  void RTEMSTimerComponentImpl::startTimer(NATIVE_INT_TYPE interval) {
      while (true) {
          Os::Task::delay(Fw::TimeInterval(static_cast<U32>(interval/1000), static_cast<U32>((interval % 1000) * 1000)));
          this->m_mutex.lock();
          bool quit = this->m_quit;
          this->m_mutex.unLock();
          if (quit) {
              return;
          }
          this->m_rawTime.now();
          this->CycleOut_out(0,this->m_rawTime);
      }
  }

} // end namespace Svc
