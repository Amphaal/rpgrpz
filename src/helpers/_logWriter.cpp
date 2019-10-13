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
    QString channel = "Default";
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
    auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz");

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

    FILE* _fs;
    auto fdp = logFilePath->toStdString();
    auto q_lfp = fdp.c_str();

    //const auto _fsErr = fopen_s(&_fs, q_lfp, mod_latest);
    _fs = fopen(q_lfp, mod_latest);
    
    if(!_fs) return; //error
    else {
        if(sessionlogToken) *sessionlogToken = true;
    }

    _fprintf_to_file(_fs, channel, context, msg);
    fclose(_fs);

}

void LogWriter::_fprintf_to_file(FILE* _fs, const QString &channel, const QMessageLogContext &context, QString msg) {

    const char * file = context.file ? context.file : "";
    const char * function = context.function ? context.function : "";

    msg = msg.replace("\n", "");
    auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz");

    fprintf(_fs, "%s %s | %s\n", 
        qUtf8Printable(currentTime), 
        qUtf8Printable(channel), 
        qUtf8Printable(msg)
    );

}
