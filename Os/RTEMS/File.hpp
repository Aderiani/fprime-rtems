#ifndef _Os_RTEMS_File_hpp_
#define _Os_RTEMS_File_hpp_

#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/StringBase.hpp>

namespace Os {

    class File {
        public:
            enum Status {
                OP_OK,              //!< Operation successful
                DOESNT_EXIST,       //!< File doesn't exist
                NO_SPACE,           //!< No space left
                NO_PERMISSION,      //!< No permission to file
                BAD_SIZE,           //!< File size error
                NOT_OPENED,         //!< File isn't opened
                FILE_EXISTS,        //!< File exists
                NOT_SUPPORTED,      //!< Operation not supported
                INVALID_MODE,       //!< Invalid mode
                INVALID_ARGUMENT,   //!< Invalid argument
                OTHER_ERROR,        //!< Other error
                MAX_STATUS          //!< Maximum value
            };

            enum Mode {
                OPEN_NO_MODE,       //!< No mode specified
                OPEN_READ,          //!< Open in read mode
                OPEN_WRITE,         //!< Open in write mode
                OPEN_CREATE,        //!< Create a file if it doesn't exist
                OPEN_SYNC_WRITE,    //!< Open in write mode, synced
                OPEN_APPEND,        //!< Append to a file
                MAX_OPEN_MODE       //!< Maximum value
            };

            File();
            virtual ~File();
            
            Status open(const Fw::StringBase& fileName, Mode mode);
            Status read(U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE& size_read);
            Status write(const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE& size_written);
            Status flush();
            Status seek(NATIVE_INT_TYPE offset, bool absolute);
            Status close();
            
            NATIVE_INT_TYPE getSize();
            
        private:
            POINTER_CAST m_handle; //!< Handle to OS file
            bool m_opened; //!< Whether file is opened
    };
}

#endif // _Os_RTEMS_File_hpp_