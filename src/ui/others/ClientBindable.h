#pragma once 

#include "src/network/rpz/client/RPZClientThread.h"
#include <QObject>

class ClientBindable {
    
    public:

        ClientBindable();
        
        static void bindAll(RPZClientThread* cc);
        static void unbindAll();

        virtual void onRPZClientThreadConnecting(RPZClientThread* cc);
        virtual void onRPZClientThreadDisconnect(RPZClientThread* cc);

        void unbindRPZClientThread();

    protected:
        RPZClientThread* _rpzClient = nullptr;
    
    private:
        static inline QVector<ClientBindable*> _boundWidgets = QVector<ClientBindable*>();
};