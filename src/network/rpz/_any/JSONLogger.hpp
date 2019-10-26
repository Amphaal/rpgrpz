#pragma once 

#include <QString>
#include <QDebug>
#include "RPZJSON.hpp"

class JSONLogger {
    public:
        JSONLogger(const QString &loggerId) : _loggerId(loggerId) {};
    
        void log(const RPZJSON::Method &method, const QString &msg) {
            qDebug() << qUtf8Printable(this->_loggerId)
                        << "-" 
                        << method
                        << "->" 
                        << qUtf8Printable(msg);
        }

        void log(const QString &msg) {
            qDebug() << qUtf8Printable(this->_loggerId)
                     << "-" 
                     << qUtf8Printable(msg);
        }


    private:
        QString _loggerId;
};