#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <QDateTime>

#include <algorithm>

#include "_const.cpp"

class msgHandler {
    public:
        static void customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg) {   
            
            //default log
            auto logFLoc = getLogFileLocation();
            auto _fs = fopen(logFLoc.c_str(), "a+");
            if(!_fs) return; //error

            //latest log
            auto mod_latest = "a+";
            if(!_latest_been_inst) {
                mod_latest = "w";
            }
            auto logLatestFLoc = getLatestLogFileLocation();
            auto _fsLatest = fopen(logLatestFLoc.c_str(), mod_latest);
            if(!_fsLatest) {
                return; //error
            } else {
                _latest_been_inst = true;
            }
        
            //channel
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

            _fprtint(channel, context, msg);

            //print to file
            _fprintf_to_file(_fs, channel, context, msg);
            fclose(_fs);

            _fprintf_to_file(_fsLatest, channel, context, msg);
            fclose(_fsLatest);
        }

    private:

        static inline bool _latest_been_inst = false;

        static void _fprtint(std::string channel, const QMessageLogContext &context, const QString &msg) {
            
            auto localMsg = msg.toStdString();
            localMsg.erase(std::remove(localMsg.begin(), localMsg.end(), '\n'), localMsg.end());

            auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz").toStdString();

            fprintf(stderr, "%s %s | %s\n", 
                currentTime.c_str(), 
                channel.c_str(), 
                localMsg.c_str()
            );
        }

        static void _fprintf_to_file(FILE* _fs, std::string channel, const QMessageLogContext &context, const QString &msg) {

            auto localMsg = msg.toStdString();
            localMsg.erase(std::remove(localMsg.begin(), localMsg.end(), '\n'), localMsg.end());

            const char * file = context.file ? context.file : "";
            const char * function = context.function ? context.function : "";
            auto currentTime = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss.zzz").toStdString();

            fprintf(_fs, "%s %s | %s\n", 
                currentTime.c_str(), 
                channel.c_str(), 
                localMsg.c_str()
            );
            
            // fprintf(_fs, "%s %s | %s | (%s:%u, %s)\n", 
            //     currentTime.c_str(), 
            //     channel.c_str(), 
            //     localMsg.c_str(), 
            //     file, 
            //     context.line, 
            //     function
            // );
        }
};