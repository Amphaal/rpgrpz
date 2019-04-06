#pragma once

#include <QtCore/QThread>
#include "libs/upnpc_custom/upnpc.hpp"
#include <QDebug>

class uPnPThread : public QThread {
    
    Q_OBJECT
        
    signals:
        void uPnPError(int errorCode);
        void uPnPExtIpFound(const std::string &extIP);
        void uPnPSuccess(const char * protocol, const char * negociatedPort);
};