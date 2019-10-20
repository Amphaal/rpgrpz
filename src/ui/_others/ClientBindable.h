#pragma once 

#include "src/network/rpz/client/RPZClient.h"
#include <QObject>

class ClientBindable {
    public:
        ClientBindable();
        
        static void bindAll(RPZClient* cc);
        static void unbindAll();

        static bool isHostAble();

        static QVector<ClientBindable*> boundWidgets();

    virtual void onRPZClientDisconnect();

    protected:
        static inline RPZClient* _rpzClient = nullptr;
        virtual void onRPZClientConnecting();
        
    private:
        static inline QVector<ClientBindable*> _boundWidgets;
        static void _onClientThreadFinished();
};

class ClientBindableMain : public QObject {

    Q_OBJECT

    public:
        static ClientBindableMain* get() {
            if(!_inst) _inst = new ClientBindableMain;
            return _inst;
        }
    
    public slots:
        void trigger() {
            for(auto w : ClientBindable::boundWidgets()) {
                w->onRPZClientDisconnect();
            }
        }

    private:
        ClientBindableMain() {}
        static inline ClientBindableMain* _inst = nullptr;
};