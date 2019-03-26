#pragma once

#include <QtCore/QThread>
#include "libs/upnpc_custom/upnpc.hpp"
#include <QDebug>

class uPnPThread : public QThread {
    
    Q_OBJECT
        
    signals:
        void initialized(int errorCode, const char * negociatedPort);
};