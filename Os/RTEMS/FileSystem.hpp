#ifndef _Os_RTEMS_FileSystem_hpp_
#define _Os_RTEMS_FileSystem_hpp_

#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/StringBase.hpp>

namespace Os {

    class FileSystem {
        public:
            enum Status {
                OP_OK,               //!< Operation successful
                ALREADY_EXISTS,      //!< File or directory already exists
                NO_SPACE,            //!< No space left
                NO_PERMISSION,       //!< No permission
                NOT_DIR,             //!< Not a directory
                IS_DIR,              //!< Is a directory
                NOT_EMPTY,           //!< Directory not empty
                INVALID_PATH,        //!< Invalid path
                DOESNT_EXIST,        //!< File or directory doesn't exist
                FILE_LIMIT,          //!< Too many files open
                BUSY,                //!< Resource busy
                NO_MORE_FILES,       //!< No more files
                BUFFER_TOO_SMALL,    //!< Buffer too small
                EXDEV_ERROR,         //!< Cross-device error
                OVERFLOW_ERROR,      //!< Overflow error
                NOT_SUPPORTED,       //!< Operation not supported
                OTHER_ERROR          //!< Other error
            };

            // Static methods for file system operations
            static Status createDirectory(const Fw::StringBase& path);
            static Status removeDirectory(const Fw::StringBase& path);
            static Status removeFile(const Fw::StringBase& path);
            static Status moveFile(const Fw::StringBase& source, const Fw::StringBase& target);
            static Status handleFileError(int errorNumber);
    };
}

#endif // _Os_RTEMS_FileSystem_hpp_