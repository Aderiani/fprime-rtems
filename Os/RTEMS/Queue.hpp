#ifndef _Os_RTEMS_Queue_hpp_
#define _Os_RTEMS_Queue_hpp_

#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/StringBase.hpp>

namespace Os {

    class Queue {
        public:
            enum Status {
                OP_OK,               //!< Operation successful
                ALREADY_CREATED,     //!< Queue already created
                EMPTY,               //!< Queue is empty
                UNINITIALIZED,       //!< Queue not initialized
                SIZE_MISMATCH,       //!< Size mismatch
                SEND_ERROR,          //!< Error sending
                RECEIVE_ERROR,       //!< Error receiving
                INVALID_PRIORITY,    //!< Invalid priority
                FULL,                //!< Queue is full
                UNKNOWN_ERROR        //!< Unknown error
            };

            enum BlockingType {
                BLOCKING,            //!< Queue will block task waiting for message
                NONBLOCKING,         //!< Queue won't block waiting for message
            };

            Queue();
            virtual ~Queue();

            Status create(const Fw::StringBase &name, NATIVE_INT_TYPE depth, NATIVE_INT_TYPE msgSize);
            Status send(const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority = 0, BlockingType block = NONBLOCKING);
            Status receive(U8* buffer, NATIVE_INT_TYPE& size, NATIVE_INT_TYPE capacity, NATIVE_INT_TYPE& priority, BlockingType block = NONBLOCKING);
            
            NATIVE_INT_TYPE getNumMsgs() const;
            NATIVE_INT_TYPE getMaxMsgs() const;
            NATIVE_INT_TYPE getMsgSize() const;

        private:
            POINTER_CAST m_handle; //!< Queue handle
            char m_name[80]; //!< Queue name
            NATIVE_INT_TYPE m_depth; //!< Queue depth
            NATIVE_INT_TYPE m_msgSize; //!< Message size
    };
}

#endif // _Os_RTEMS_Queue_hpp_