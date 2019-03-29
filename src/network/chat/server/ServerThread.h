#pragma once

#include <QtCore/QThread>
#include <string>

class ServerThread : public QThread { 
    
    Q_OBJECT

    signals:
        void newConnectionReceived(std::string ip);

};