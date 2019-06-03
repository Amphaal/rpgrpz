#pragma once

#include "base/Stampable.hpp"

class RPZResponse : public Stampable {
    public:
        enum ResponseCode {
            UnknownCommand, 
            HelpManifest, 
            Ack, 
            Error, 
            ConnectedToServer, 
            Status
        };
        
        RPZResponse() : Stampable() {}
        RPZResponse(const QUuid &answererTo, const ResponseCode &code = Ack, const QVariant &data = QVariant()) : Stampable() {
            this->_setResponseCode(code);
            if(!answererTo.isNull()) this->_setAnswerer(answererTo);
            if(!data.isNull()) this->_setResponseData(data);
        }
        RPZResponse(const QVariantHash &hash) : Stampable(hash) {}

        ResponseCode responseCode() {
            return (ResponseCode)this->value("r").toInt();
        }

        QVariant responseData() {
            return this->value("rdata");
        }
        
        QUuid answerer() {
            return this->value("aswr").toUuid();
        }

        QString toString() override {
            
            switch(this->responseCode()) {

                case UnknownCommand: {
                    return "Le serveur n'a pas compris. Faites \"/h\" pour obtenir de l'aide !";
                }

                case ConnectedToServer: {
                    return QString("Connecté au serveur (") + this->responseData().toString() + ")";
                }

                case Error: {
                    return QString("Une erreur est survenue : ") + this->responseData().toString();
                }

                default: {
                    return this->responseData().toString();
                }
            };

        };

    private:
        void _setAnswerer(const QUuid &answererStampableId) {
            (*this)["aswr"] = answererStampableId;
        }

        void _setResponseCode(const ResponseCode &code) {
            (*this)["r"] = (int)code;
        }

        void _setResponseData(const QVariant &data) {
            (*this)["rdata"] = data;
        }

};