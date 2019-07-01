#include "_logWriter.h"

void LogWriter::customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg) {   
    
    //default log
    const auto logFLoc = AppContext::getLogFileLocation().toStdString().c_str();
    FILE* _fs;
    const auto _fsErr = fopen_s(&_fs, logFLoc, "a+");
    if(!_fsErr) return; //error

    //latest log
    auto mod_latest = "a+";
    if(!_latest_been_inst) {
        mod_latest = "w";
    }
    const auto logLatestFLoc = AppContext::getLatestLogFileLocation().toStdString().c_str();
    FILE* _fsLatest;
    const auto _fsLatestErr = fopen_s(&_fsLatest, logLatestFLoc, mod_latest);
    if(!_fsLatestErr) {
        return; //error
    } else {
        _latest_been_inst = true;
    }

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

    _fprtint(channel, context, msg);

    //print to file
    _fprintf_to_file(_fs, channel, context, msg);
    fclose(_fs);

    _fprintf_to_file(_fsLatest, channel, context, msg);
    fclose(_fsLatest);
}


void LogWriter::_fprtint(const QString &channel, const QMessageLogContext &context, const QString &msg) {
    
    auto localMsg = msg.toStdString();
    localMsg.erase(std::remove(localMsg.begin(), localMsg.end(), '\n'), localMsg.end());

    const auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz").toStdString();

    fprintf(stderr, "%s %s | %s\n", 
        currentTime.c_str(), 
        channel.toStdString().c_str(), 
        localMsg.c_str()
    );
}

void LogWriter::_fprintf_to_file(FILE* _fs, const QString &channel, const QMessageLogContext &context, const QString &msg) {

    auto localMsg = msg.toStdString();
    localMsg.erase(std::remove(localMsg.begin(), localMsg.end(), '\n'), localMsg.end());

    const char * file = context.file ? context.file : "";
    const char * function = context.function ? context.function : "";
    const auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz").toStdString();

    fprintf(_fs, "%s %s | %s\n", 
        currentTime.c_str(), 
        channel.toStdString().c_str(), 
        localMsg.c_str()
    );
}
