/*********************************************************************
    logger.cpp													  
    Copyright (c) Thomas Weber. All rights reserved.				
    Licensed under the MIT License.
    https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "logger.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <mutex>
#include <atomic>

//-----------------------------------------------------------------------------
// Name: logger()
// Desc: constructor, specify log level, log type and filename
//-----------------------------------------------------------------------------
logger::logger(logLevel level, logType type, const std::wstring &filename)
{
    m_level = level;
    m_type = type;
    m_filename = filename;
    stream = &std::wcout;
    if (m_type == logType::file || m_type == logType::both) {
        if (m_filename.empty()) {
            throw std::runtime_error("Logger filename is empty.");
        }
        m_file.open(m_filename.c_str(), std::ios::out | std::ios::app);
        if (!m_file.is_open()) {
            throw std::runtime_error("Failed to open log file.");
        }
    }
}

//-----------------------------------------------------------------------------
// Name: logger()
// Desc: default constructor
//-----------------------------------------------------------------------------
logger::logger()
{
    m_level = logLevel::info;
    m_type = logType::console;
    stream = &std::wcout;
}

//-----------------------------------------------------------------------------
// Name: logger()
// Desc: move constructor
//-----------------------------------------------------------------------------
logger::logger(logger&& other) noexcept
{
    m_level     = other.m_level;
    m_type      = other.m_type;
    m_filename  = other.m_filename;
    m_file      = std::move(other.m_file);
}

//-----------------------------------------------------------------------------
// Name: operator=()
// Desc: move assignment operator
//-----------------------------------------------------------------------------
logger& logger::operator=(logger&& other) noexcept
{
    if (this != &other)
    {
        m_level     = other.m_level;
        m_type      = other.m_type;
        m_filename  = other.m_filename;
        m_file      = std::move(other.m_file);
    }
    return *this;
}

//-----------------------------------------------------------------------------
// Name: ~logger()
// Desc: destructor
//-----------------------------------------------------------------------------
logger::~logger()
{
    // The destroyer_flag is used to ensure that the log file is closed only once,
    // even if multiple logger instances are destroyed, preventing double-close issues.
    static std::atomic<bool> destroyed_flag{false};
    if (!destroyed_flag.exchange(true)) {
        if ((m_type == logType::file || m_type == logType::both) && m_file.is_open()) {
            m_file.close();
        }
    }
    destroyed = true;
}

//-----------------------------------------------------------------------------
// Name: setStream()
// Desc: set the output stream
//-----------------------------------------------------------------------------
bool logger::setOutputStream(std::wostream& stream)
{
    if (!stream) return false;
    if (!stream.good()) return false;
    std::lock_guard<std::mutex> lock(m_mutex);
    this->stream = &stream;
    return true;
}

//-----------------------------------------------------------------------------
// Name: setLevel()
// Desc: set the log level
//-----------------------------------------------------------------------------
bool logger::setLevel(logLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
    return true;
}

//-----------------------------------------------------------------------------
// Name: log()
// Desc: Log a message
//-----------------------------------------------------------------------------
bool logger::log(logLevel level, const std::wstring &message, bool newLine, bool begin)
{
    if (destroyed) return false;
    if (level <= m_level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_type == logType::console || m_type == logType::both) {
            *stream << (begin ? getLogString(level, message, L"", L"", 0) : message) << (newLine ? L"\n" : L"") << std::flush;
        }

        if (m_type == logType::file || m_type == logType::both) {
            m_file << (begin ? getLogString(level, message, L"", L"", 0) : message) << (newLine ? L"\n" : L"") << std::flush;
        }
    }
    return getReturnValue(level);
}

//-----------------------------------------------------------------------------
// Name: log()
// Desc: Log a message with function, file and line information
//-----------------------------------------------------------------------------
bool logger::log(logLevel level, const std::wstring &message, const std::wstring &function, const std::wstring &file, int line)
{
    if (destroyed) return false;
    if (level <= m_level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_type == logType::console || m_type == logType::both) {
            *stream << getLogString(level, message, function, file, line) << std::endl;
        }

        if (m_type == logType::file || m_type == logType::both) {
            m_file << getLogString(level, message, function, file, line) << std::endl;
        }
    }
    return getReturnValue(level);
}

//-----------------------------------------------------------------------------
// Name: getLevelString()
// Desc: Get the string representation of the log level
//-----------------------------------------------------------------------------
std::wstring logger::getLevelString(logLevel level)
{
    switch (level)
    {
    case logLevel::error:
        return L"ERROR";
    case logLevel::warning:
        return L"WARN ";
    case logLevel::info:
        return L"INFO ";
    case logLevel::debug:
        return L"DEBUG";
    case logLevel::trace:
        return L"TRACE";
    default:
        return L"";
    }
}

//-----------------------------------------------------------------------------
// Name: getTimeString()
// Desc: Get the current time as a string
//-----------------------------------------------------------------------------
std::wstring logger::getTimeString()
{
    std::time_t t = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &t);

    std::wstringstream ss;
    ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
    return ss.str();
}

//-----------------------------------------------------------------------------
// Name: getLogString()
// Desc: Get the log message as a string
//-----------------------------------------------------------------------------
std::wstring logger::getLogString(logLevel level, const std::wstring &message, const std::wstring &function, const std::wstring &file, int line)
{
    std::wstringstream ss;
    ss << getLevelString(level) << L" " << getTimeString() << L" " << message;

    if (!function.empty()) {
        ss << L" (" << function << L" in " << file << L" at line " << line << L")";
    }

    return ss.str();
}

//-----------------------------------------------------------------------------
// Name: getReturnValue()
// Desc: Get the return value for the log message
//-----------------------------------------------------------------------------
bool logger::getReturnValue(logLevel level)
{
    switch (level)
    {
    case logLevel::error:
        return false;
    case logLevel::warning:
    case logLevel::info:
    case logLevel::debug:
    case logLevel::trace:
        return true;
    default:
        return true;
    }
}
