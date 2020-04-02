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

#include "_logWriter.h"

QString* LogWriter::_getFullLogFilePath() {
    if(!_fullLogFilePath) {
        _fullLogFilePath = new QString(AppContext::getLogFileLocation());
    }
    return _fullLogFilePath;
}

QString* LogWriter::_getSessionLogFilePath() {
    if(!_sessionLogFilePath) {
        _sessionLogFilePath = new QString(AppContext::getLatestLogFileLocation());
    }
    return _sessionLogFilePath;
}

void LogWriter::customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg) {   
    
    //lock
    std::unique_lock<std::mutex> lock(_m);

    //channel
    QString channel = QStringLiteral(u"Default");
    switch (type) {
        case QtDebugMsg:
            channel = QStringLiteral(u"Debug");
            break;
        case QtInfoMsg:
            channel = QStringLiteral(u"Info");
            break;
        case QtWarningMsg:
            channel = QStringLiteral(u"Warning");
            break;
        case QtCriticalMsg:
            channel = QStringLiteral(u"Critical");
            break;
        case QtFatalMsg:
            channel = QStringLiteral(u"Fatal");
            break;
    }

    //std console print
    _fprtint(channel, context, msg);

    //full log
    auto flfp = _getFullLogFilePath();
    _openFileAndLog(flfp, channel, context, msg);

    //session log
    auto slfp = _getSessionLogFilePath();
    _openFileAndLog(slfp, channel, context, msg, &_latest_been_inst);
}


void LogWriter::_fprtint(const QString &channel, const QMessageLogContext &context, QString msg) {
    
    msg = msg.replace("\n", "");
    auto currentTime = QDateTime::currentDateTime().toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss.zzz"));

    fprintf(stderr, "%s %s | %s\n", 
        qUtf8Printable(currentTime), 
        qUtf8Printable(channel), 
        qUtf8Printable(msg)
    );

}

void LogWriter::_openFileAndLog(QString* logFilePath, const QString &channel, const QMessageLogContext &context, const QString &msg, bool* sessionlogToken) {

    //latest log
    auto mod_latest = "a+";
    if(sessionlogToken && !*sessionlogToken) {
        mod_latest = "w";
    }

    //const auto _fsErr = fopen_s(&_fs, q_lfp, mod_latest);
    auto _fs = fopen(qUtf8Printable(*logFilePath), mod_latest);
    
    if(!_fs) return; //error
    else {
        if(sessionlogToken) *sessionlogToken = true;
    }

    _fprintf_to_file(_fs, channel, context, msg);
    fclose(_fs);

}

void LogWriter::_fprintf_to_file(FILE* _fs, const QString &channel, const QMessageLogContext &context, QString msg) {

    // const char * file = context.file ? context.file : "";
    // const char * function = context.function ? context.function : "";

    msg = msg.replace("\n", "");
    auto currentTime = QDateTime::currentDateTime().toString(QStringLiteral(u"dd.MM.yyyy-hh:mm:ss.zzz"));

    fprintf(_fs, "%s %s | %s\n", 
        qUtf8Printable(currentTime), 
        qUtf8Printable(channel), 
        qUtf8Printable(msg)
    );

}
