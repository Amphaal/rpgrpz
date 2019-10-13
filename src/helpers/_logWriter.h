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