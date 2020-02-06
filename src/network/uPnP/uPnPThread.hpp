#pragma once

#include <QThread>
#include <QDebug>

//.hpp for QT bindings to generate
class uPnPThread : public QThread {
    
    Q_OBJECT
        
    signals:
        void uPnPError(int errorCode);
        void uPnPExtIpFound(const QString &extIP);
        void uPnPSuccess(const QString &protocol, const QString &negociatedPort);
        
};