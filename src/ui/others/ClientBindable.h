#pragma once 

#include "src/network/rpz/client/RPZClient.h"
#include <QObject>

class ClientBindable {
    
    public:

        ClientBindable();
        
        static void bindAll(RPZClient* cc);
        static void unbindAll();

        virtual void onRPZClientConnecting(RPZClient* cc);
        virtual void onRPZClientDisconnect(RPZClient* cc);

        void unbindRPZClient();

    protected:
        RPZClient* _rpzClient = nullptr;
    
    private:
        static inline QVector<ClientBindable*> _boundWidgets = QVector<ClientBindable*>();
};