// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "_logWriter.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

void LogWriter::initFromContext() {
    //
    _fullLogFilePath = AppContext::getLogFileLocation().toStdString();
    _sessionLogFilePath = AppContext::getLatestLogFileLocation().toStdString();

    // define default spdlog behavior
    spdlog::default_logger()->set_level(spdlog::level::trace);
    spdlog::default_logger()->flush_on(spdlog::level::trace);

    // add file sinks
    auto &sinks = spdlog::default_logger()->sinks();
    auto fullLog_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(_fullLogFilePath);
    auto sessionLog_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(_sessionLogFilePath);
    sinks.push_back(fullLog_sink);
    sinks.push_back(sessionLog_sink);
}


void LogWriter::sentryLogHandler(sentry_level_t level, const char *message, va_list args, void *userdata) {
    // channel
    std::string channel = "Default";
    switch (level){
        case SENTRY_LEVEL_DEBUG :
            channel = "Debug";
        break;
        case SENTRY_LEVEL_INFO :
            channel = "Info";
        break;
        case SENTRY_LEVEL_WARNING : 
            channel = "Warning";
        break;
        case SENTRY_LEVEL_ERROR : 
            channel = "Error";
        break;
        case SENTRY_LEVEL_FATAL : 
            channel = "Fatal";
        break;
    }

    //
    vsnprintf(_sentryLogBuff, sizeof(_sentryLogBuff), message, args);
    _sentryLogStr = _sentryLogBuff;
    _sentryLogStr.prepend("[sentry] ");

    //
    _fillSinks(channel.c_str(), _sentryLogStr);
}

void LogWriter::customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // channel
    std::string channel = "Default";
    switch (type) {
        case QtDebugMsg:
            channel = "Debug";
            break;
        case QtInfoMsg:
            channel = "Info";
            break;
        case QtWarningMsg:
            channel = "Warning";
            break;
        case QtCriticalMsg:
            channel = "Critical";
            break;
        case QtFatalMsg:
            channel = "Fatal";
            break;
    }
    
    //
    _fillSinks(channel.c_str(), msg);
}

void LogWriter::_fillSinks(const char* channel, const QString &msg) {
    // lock
    std::unique_lock<std::mutex> lock(_m);
    
    // std console print
    _fprintf(channel, msg);

    // full log
    _openFileAndLog(_fullLogFilePath.c_str(), channel, msg);

    // session log
    _openFileAndLog(_sessionLogFilePath.c_str(), channel, msg, &_latest_been_inst);
}

void LogWriter::_openFileAndLog(const char* logFilePath, const char* channel,const QString &msg, bool* sessionlogToken) {
    // latest log
    auto mod_latest = "a+";
    if (sessionlogToken && !*sessionlogToken) {
        mod_latest = "w";
    }

    // open file
    auto _fs = fopen(logFilePath, mod_latest);

    if (!_fs) return;  // error
    if (sessionlogToken) *sessionlogToken = true;

    // print to file
    _fprintf(channel, msg, _fs);

    // close file
    fclose(_fs);
}

void LogWriter::_fprintf(const char* channel, const QString &msg, FILE* _fs) {
    if(!_fs) _fs = stdout;
    
    auto currentTime = QDateTime::currentDateTime()
        .toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss.zzz"));

    fprintf(_fs, "%s %s | %s\n",
        qUtf8Printable(currentTime),
        qUtf8Printable(channel),
        qUtf8Printable(msg)
    );
}
