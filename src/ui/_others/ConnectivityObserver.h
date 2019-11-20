#pragma once 

#include <QObject>

#include "src/network/rpz/client/RPZClient.h"
#include "src/helpers/Authorisations.hpp"

class ConnectivityObserver {
    public:
        ConnectivityObserver();
                
        static void connectWithClient(RPZClient* cc);
        static void disconnectClient();

        static const QVector<ConnectivityObserver*> observers();

        void receivedConnectionCloseSignal(bool hasInitialMapLoaded);

    protected:
        static inline RPZClient* _rpzClient = nullptr;
        virtual void connectingToServer() {}
        virtual void connectionClosed(bool hasInitialMapLoaded) {}

    private:
        static inline QVector<ConnectivityObserver*> _observers;
        static void _onClientThreadFinished();


};

class ConnectivityObserverSynchronizer : public QObject {

    Q_OBJECT

    public:
        static ConnectivityObserverSynchronizer* get();
    
    public slots:
        void triggerConnectionClosed(bool hasInitialMapLoaded);

    private:
        ConnectivityObserverSynchronizer() {}
        static inline ConnectivityObserverSynchronizer* _inst = nullptr;
        
};