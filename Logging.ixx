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
    export class ConsoleFormatter {
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
    };
}
