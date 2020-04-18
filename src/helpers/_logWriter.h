// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

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
#include <QDateTime>
#include <mutex>

#include <algorithm>

#include "_appContext.h"

class LogWriter {
    public:
        static void customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    private:
        static inline std::mutex _m;
        
        static void _fprtint(const QString &channel, const QMessageLogContext &context, QString msg);
        static void _openFileAndLog(QString* logFilePath, const QString &channel, const QMessageLogContext &context, const QString &msg, bool* sessionlogToken = nullptr);
            static void _fprintf_to_file(FILE* _fs, const QString &channel, const QMessageLogContext &context, QString msg);

        static inline QString* _fullLogFilePath = nullptr;
        static QString* _getFullLogFilePath();

        static inline bool _latest_been_inst = false;
        static inline QString* _sessionLogFilePath = nullptr;
        static QString* _getSessionLogFilePath();
};