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

namespace Svc {

  // ----------------------------------------------------------------------
  // Construction, initialization, and destruction
  // ----------------------------------------------------------------------

  RTEMSTimerComponentImpl ::
    RTEMSTimerComponentImpl(
        const char *const compName
    ) : RTEMSTimerComponentBase(compName),
        m_quit(false)
  {

  }

  RTEMSTimerComponentImpl ::
    ~RTEMSTimerComponentImpl()
  {

  }

  void RTEMSTimerComponentImpl::quit() {
      this->m_mutex.lock();
      this->m_quit = true;
      this->m_mutex.unLock();
  }

} // end namespace Svc
