module;

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Record.h>
#include <iomanip>

export module Logging;

import std;

namespace plog {
    class ConsoleFormatter {
    public:
        static util::nstring header() {
            return util::nstring();
        }

        static util::nstring format(const Record& record) {
            tm t;
            util::localtime_s(&t, &record.getTime().time);

            util::nostringstream ss;
            ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_year + 1900 << PLOG_NSTR("-")
               << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
            ss << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setw(2) << t.tm_min << PLOG_NSTR(":")
               << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setw(3)
               << static_cast<int>(record.getTime().millitm) << PLOG_NSTR(" ");
            ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity())
               << PLOG_NSTR(" ");
            ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
            ss << record.getMessage() << PLOG_NSTR("\n");

            return ss.str();
        }
    };
}

namespace OpenKaiser {

    export enum class LogSeverity {
        None = 0,
        Fatal = 1,
        Error = 2,
        Warning = 3,
        Info = 4,
        Debug = 5,
        Verbose = 6
    };

    export class LogStream {
    public:
        LogStream(plog::Severity severity, std::source_location location = std::source_location::current()) 
            : severity_(severity), location_(location) {}

        ~LogStream() {
            if (!moved_ && !stream_.str().empty()) {
                plog::Record record(severity_, location_.function_name(), location_.line(), 
                                   location_.file_name(), nullptr, 0);
                record.ref() << stream_.str();
                *plog::get() += record;
            }
        }

        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;

        LogStream(LogStream&& other) noexcept 
            : severity_(other.severity_), 
              location_(other.location_),
              stream_(std::move(other.stream_)) {
            other.moved_ = true;
        }

        template<typename T>
        LogStream& operator<<(T&& value) {
            if (!moved_) {
                stream_ << std::forward<T>(value);
            }
            return *this;
        }

        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (!moved_) {
                stream_ << manip;
            }
            return *this;
        }

    private:
        plog::Severity severity_;
        std::source_location location_;
        std::ostringstream stream_;
        bool moved_ = false;
    };

    export class Logger {
    public:
        static void InitConsole(LogSeverity maxSeverity = LogSeverity::Info) {
            static plog::ColorConsoleAppender<plog::ConsoleFormatter> consoleAppender;
            plog::init(static_cast<plog::Severity>(maxSeverity), &consoleAppender);
        }

        static void InitFile(const std::string& fileName, 
                           LogSeverity maxSeverity = LogSeverity::Info,
                           size_t maxFileSize = 1000000,
                           int maxFiles = 3) {
            static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(
                fileName.c_str(), maxFileSize, maxFiles);
            plog::init(static_cast<plog::Severity>(maxSeverity), &fileAppender);
        }

        static void InitBoth(const std::string& fileName,
                           LogSeverity maxSeverity = LogSeverity::Info,
                           size_t maxFileSize = 1000000,
                           int maxFiles = 3) {
            static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(
                fileName.c_str(), maxFileSize, maxFiles);
            static plog::ColorConsoleAppender<plog::ConsoleFormatter> consoleAppender;

            plog::init(static_cast<plog::Severity>(maxSeverity), &fileAppender)
                .addAppender(&consoleAppender);
        }

        template<typename... Args>
        static void Verbose(Args&&... args) {
            LogMessage(plog::verbose, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Debug(Args&&... args) {
            LogMessage(plog::debug, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Info(Args&&... args) {
            LogMessage(plog::info, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Warning(Args&&... args) {
            LogMessage(plog::warning, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Error(Args&&... args) {
            LogMessage(plog::error, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Fatal(Args&&... args) {
            LogMessage(plog::fatal, std::forward<Args>(args)...);
        }

        static LogStream Verbose(std::source_location location = std::source_location::current()) { 
            return LogStream(plog::verbose, location); 
        }

        static LogStream Debug(std::source_location location = std::source_location::current()) { 
            return LogStream(plog::debug, location); 
        }

        static LogStream Info(std::source_location location = std::source_location::current()) { 
            return LogStream(plog::info, location); 
        }

        static LogStream Warning(std::source_location location = std::source_location::current()) { 
            return LogStream(plog::warning, location); 
        }

        static LogStream Error(std::source_location location = std::source_location::current()) { 
            return LogStream(plog::error, location); 
        }

        static LogStream Fatal(std::source_location location = std::source_location::current()) { 
            return LogStream(plog::fatal, location); 
        }

    private:
        template<typename... Args>
        static void LogMessage(plog::Severity severity, Args&&... args) {
            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));
            PLOG(severity) << oss.str();
        }
    };
}
