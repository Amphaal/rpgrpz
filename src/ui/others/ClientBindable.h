#pragma once 

#include "src/network/rpz/client/RPZClient.h"
#include <QObject>

class ClientBindable {
    
    public:
        ClientBindable();
        
        static void bindAll(RPZClient* cc);
        static void unbindAll();

    protected:
        static inline RPZClient* _rpzClient = nullptr;
        virtual void onRPZClientConnecting();
        virtual void onRPZClientDisconnect();

    private:
        static inline QVector<ClientBindable*> _boundWidgets;
        static void _onClientThreadFinished();
};