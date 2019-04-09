#pragma once 

#include "src/network/rpz/client/RPZClient.h"
#include <QObject>

class ClientBindable {
    
    public:
        virtual void bindToRPZClient(RPZClient* cc) {
            
            this->_rpzClient = cc;

            //pass to nullptr for comparaisons
            QObject::connect(
                this->_rpzClient, &QObject::destroyed,
                [&]() {
                    this->_rpzClient = nullptr;
                }
            );

        }

        void unbindRPZClient() {
            if(this->_rpzClient) {
                this->_rpzClient->disconnect();
            }
        }

    protected:
        RPZClient* _rpzClient = nullptr;
};