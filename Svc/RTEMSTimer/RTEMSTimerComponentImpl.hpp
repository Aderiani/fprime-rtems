// ======================================================================
// \title  RTEMSTimerImpl.hpp
// \author tim
// \brief  hpp file for RTEMSTimer component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef RTEMSTimer_HPP
#define RTEMSTimer_HPP

#include "Os/Mutex.hpp"
#include "Svc/RTEMSTimer/RTEMSTimerComponentAc.hpp"

namespace Svc {

  class RTEMSTimerComponentImpl final :
    public RTEMSTimerComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object RTEMSTimer
      //!
      RTEMSTimerComponentImpl(
          const char *const compName /*!< The component name*/
      );

      //! Destroy object RTEMSTimer
      //!
      ~RTEMSTimerComponentImpl();

      //! Start timer
      void startTimer(NATIVE_INT_TYPE interval); //!< interval in milliseconds

      //! Quit timer
      void quit();

    PRIVATE:

      Os::Mutex m_mutex; //!< mutex for quit flag

      volatile bool m_quit; //!< flag to quit

      Os::RawTime m_rawTime; //!< timestamp to pass to CycleOut port calls


    };

} // end namespace Svc

#endif
