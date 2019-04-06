#pragma once

#include <QtCore/QThread>
#include <string>

class RPZSThread : public QThread { 
    
    Q_OBJECT

    signals:
        void newConnectionReceived(const std::string &ip);

};