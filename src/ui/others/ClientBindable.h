#pragma once 

#include "src/network/rpz/client/RPZClientThread.h"
#include <QObject>

class ClientBindable {
    
    public:
        ClientBindable();
        
        static void bindAll(RPZClientThread* cc);
        static void unbindAll();

    protected:
        static inline RPZClientThread* _rpzClient = nullptr;
        virtual void onRPZClientThreadConnecting();
        virtual void onRPZClientThreadDisconnect();

    private:
        static inline QVector<ClientBindable*> _boundWidgets;
        static void _onClientThreadFinished();
};