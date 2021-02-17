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

#pragma once

#include <QApplication>
#include <stdio.h>
#include <stdlib.h>

#include <sentry.h>

#include <QDateTime>

#include <algorithm>

#include "_appContext.h"

class LogWriter {
 public:
    static void initFromContext();

    static void customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void sentryLogHandler(sentry_level_t level, const char *message, va_list args, void *userdata);

 private:
    static inline char _sentryLogBuff[512];
    static inline QString _sentryLogStr;

    static inline std::mutex _m;

    static void _fillSinks(const char* channel, const QString &msg);
    static void _fprintf(const char* channel, const QString &msg, FILE* _fs = nullptr);

    static void _openFileAndLog(
        const char* logFilePath,
        const char* channel,
        const QString &msg,
        bool* sessionlogToken = nullptr);

    static inline bool _latest_been_inst = false;
    static inline std::string _fullLogFilePath;
    static inline std::string _sessionLogFilePath;
};
