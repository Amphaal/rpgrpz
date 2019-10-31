#pragma once 

#include <QObject>

#include "src/network/rpz/client/RPZClient.h"

class ConnectivityObserver {
    public:
        ConnectivityObserver();
                
        static void bindAll(RPZClient* cc);
        static void unbindAll();

        static const QVector<ConnectivityObserver*> observers();

        void receivedConnectionCloseSignal();

    protected:
        static inline RPZClient* _rpzClient = nullptr;
        virtual void connectingToServer() {}
        virtual void connectionClosed() {}

    private:
        static inline QVector<ConnectivityObserver*> _observers;
        static void _onClientThreadFinished();


};

class ConnectivityObserverSynchronizer : public QObject {

    Q_OBJECT

    public:
        static ConnectivityObserverSynchronizer* get();
    
    public slots:
        void triggerConnectionClosed();

    private:
        ConnectivityObserverSynchronizer() {}
        static inline ConnectivityObserverSynchronizer* _inst = nullptr;
        
};