#include "_logWriter.h"

void LogWriter::customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg) {   
    
    //default log
    const auto logFLoc = AppContext::getLogFileLocation();
    const auto _fs = fopen(logFLoc.toStdString().c_str(), "a+");
    if(!_fs) return; //error

    //latest log
    auto mod_latest = "a+";
    if(!_latest_been_inst) {
        mod_latest = "w";
    }
    const auto logLatestFLoc = AppContext::getLatestLogFileLocation();
    const auto _fsLatest = fopen(logLatestFLoc.toStdString().c_str(), mod_latest);
    if(!_fsLatest) {
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
