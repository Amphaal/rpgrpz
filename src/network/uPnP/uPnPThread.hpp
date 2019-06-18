#pragma once

#include <QtCore/QThread>
#include "src/_libs/upnpc_custom/upnpc.hpp"
#include <QDebug>

//.hpp for QT bindings to generate
class uPnPThread : public QThread {
    
    Q_OBJECT
        
    signals:
        void uPnPError(int errorCode);
        void uPnPExtIpFound(const QString &extIP);
        void uPnPSuccess(const char * protocol, const char * negociatedPort);
        
};