/*********************************************************************\
    logger.h													  
    Copyright (c) Thomas Weber. All rights reserved.				
    Licensed under the MIT License.
    https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <mutex>

// logger class
// - logs messages to the console, a file or both
// - the log level can be set to none, error, warning, info, debug or trace
// - the log type can be set to none, console, file or both
// - the filename is only used if the log type is set to file or both
// - the logger can be used like a stream, e.g. log << "message" << std::endl;
// - thread safe
class logger
{
public:
    // enumerations
    enum class logLevel { none, error, warning, info, debug, trace };
    enum class logType  { none, console, file, both };

    // constructors and destructor
                logger          ();
                logger          (logLevel level, logType type, const std::wstring &filename);
                logger          (logger&& other) noexcept;
                ~logger         ();

    // functions
    bool        setOutputStream (std::wostream& stream);
    bool        setLevel        (logLevel level);
    logLevel    getLevel        () const { return m_level; }
    logType     getType         () const { return m_type; }
    bool        log             (logLevel level, const std::wstring &message, bool newLine = true, bool begin = true);
    bool        log             (logLevel level, const std::wstring &message, const std::wstring &function, const std::wstring &file, int line);
    logger&     operator=       (logger&& other) noexcept;

    // stream operator
    template<typename T> 
    friend logger& operator<<(logger& log, const T& data) 
    {
        if (log.destroyed) return log;
        std::wstringstream wss;
        wss << data;
        // if we are at the beginning of a new line, then we need to log header information
        if (log.next_is_begin) {
            log.log(logLevel::info, wss.str(), false, true);
            log.next_is_begin = false;
        } else {
            log.log(logLevel::info, wss.str(), false, false);
        }
        // if the string ends with a newline, then the next string will be at the beginning of a new line
        if (wss.str().ends_with(L"\n")) {
            log.next_is_begin = true;
        }
        return log;
    }

private:
    logLevel        m_level         = logLevel::info;
    logType         m_type          = logType::console;
    std::wstring    m_filename;
    std::wofstream  m_file;
    std::mutex      m_mutex;
    bool            next_is_begin   = true;
    std::wostream*  stream          = &std::wcout;
    bool            destroyed       = false; 

    std::wstring    getLevelString  (logLevel level);
    std::wstring    getTimeString   ();
    std::wstring    getLogString    (logLevel level, const std::wstring &message, const std::wstring &function, const std::wstring &file, int line);
    bool            getReturnValue  (logLevel level);
};

#endif