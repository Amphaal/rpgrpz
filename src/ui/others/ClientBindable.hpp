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
                widget->bindToRPZClient(cc);
            }
        }

        static void unbindAll() {
            for(auto widget : _boundWidgets) {
                widget->unbindRPZClient();
            }
        }

        virtual void bindToRPZClient(RPZClient* cc) {
            
            this->unbindRPZClient();
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
    
    private:
        static inline QVector<ClientBindable*> _boundWidgets = QVector<ClientBindable*>();
};