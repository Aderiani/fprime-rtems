#ifndef _Os_RTEMS_Directory_hpp_
#define _Os_RTEMS_Directory_hpp_

#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/StringBase.hpp>

namespace Os {

    class Directory {
        public:
            enum Status {
                OP_OK,              //!< Operation successful
                DOESNT_EXIST,       //!< Directory doesn't exist
                NO_PERMISSION,      //!< No permission
                NOT_OPENED,         //!< Directory not opened
                NOT_DIR,            //!< Not a directory
                NO_MORE_FILES,      //!< No more files in directory
                FILE_LIMIT,         //!< Too many files opened
                BAD_DESCRIPTOR,     //!< Bad file descriptor
                ALREADY_EXISTS,     //!< File already exists
                NOT_SUPPORTED,      //!< Operation not supported
                OTHER_ERROR         //!< Other error
            };

            enum OpenMode {
                READ,               //!< Open for reading
                CREATE_IF_MISSING,  //!< Create directory if missing
                CREATE_EXCLUSIVE,   //!< Create only if does not exist
                MAX_OPEN_MODE       //!< Maximum value
            };

            Directory();
            virtual ~Directory();
            
            Status createDirectory(const Fw::StringBase& path);
            Status removeDirectory(const Fw::StringBase& path);
            Status readDirectory(const Fw::StringBase& path, Fw::String& entry);
            Status openDirectory(const Fw::StringBase& path, OpenMode mode = READ);
            Status closeDirectory();
            
        private:
            POINTER_CAST m_handle; //!< Handle to OS directory
            bool m_opened; //!< Whether directory is opened
    };
}

#endif // _Os_RTEMS_Directory_hpp_