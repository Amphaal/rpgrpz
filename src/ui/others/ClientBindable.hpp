#pragma once 

#include "src/network/rpz/client/RPZClient.h"
#include <QObject>

class ClientBindable {
    
    public:

        ClientBindable() {
            _boundWidgets.append(this);
        }
        
        static void bindAll(RPZClient* cc) {
            for(auto widget : _boundWidgets) {
                widget->onRPZClientConnecting(cc);
            }
        }

        static void unbindAll() {
            for(auto widget : _boundWidgets) {
                widget->unbindRPZClient();
            }
        }

        virtual void onRPZClientConnecting(RPZClient* cc) {
            
            this->unbindRPZClient();
            this->_rpzClient = cc;

            //on disconnect
            QObject::connect(
                this->_rpzClient, &JSONSocket::disconnected,
                [&]() {
                    this->onRPZClientDisconnect(this->_rpzClient);
                }
            );

            //pass to nullptr for comparaisons
            QObject::connect(
                this->_rpzClient, &QObject::destroyed,
                [&]() {
                    this->_rpzClient = nullptr;
                }
            );

        }

        virtual void onRPZClientDisconnect(RPZClient* cc) {}

        void unbindRPZClient() {
            if(this->_rpzClient) {
                this->_rpzClient->disconnect();
            }
        }

    protected:
        RPZClient* _rpzClient = nullptr;
    
    private:
        static inline QVector<ClientBindable*> _boundWidgets = QVector<ClientBindable*>();
};