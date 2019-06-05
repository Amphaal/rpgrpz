#pragma once

#include "base/Stampable.hpp"

class RPZResponse : public Stampable {
    public:
        enum ResponseCode {
            UnknownCommand, 
            HelpManifest, 
            Ack, 
            Error,
            ErrorRecipients,
            ConnectedToServer, 
            Status
        };
        
        RPZResponse() : Stampable() {}
        RPZResponse(const snowflake_uid &answererTo, const ResponseCode &code = Ack, const QVariant &data = QVariant()) : Stampable() {
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
        
        snowflake_uid answerer() {
            return this->value("aswr").toULongLong();
        }

        QString toString() override {
            
            switch(this->responseCode()) {

                case UnknownCommand: {
                    return "Le serveur n'a pas compris. Faites \"/h\" pour obtenir de l'aide !";
                }

                case ErrorRecipients: {

                    QList<QString> rcpts;
                    for(auto &e : this->responseData().toList()) rcpts.append(e.toString());

                    return QString("Les utilisateurs cibles n'ont pas été trouvés : ") + rcpts.join(", ");
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

        QPalette palette() override {
            
            //default palette
            auto palette = Stampable::palette();

            //switch by resp code...
            switch(this->responseCode()) {
                
                case Error:
                case ErrorRecipients:
                    palette.setColor(QPalette::Window, "#f9dad4");
                    palette.setColor(QPalette::WindowText, "#FF0000");
                    break;

                case ConnectedToServer:
                case Status:
                    palette.setColor(QPalette::Window, "#71ed55");
                    palette.setColor(QPalette::WindowText, "#0f4706");
                    break;

            }

            return palette;
        }

    private:
        void _setAnswerer(const snowflake_uid &answererStampableId) {
            (*this)["aswr"] = QString::number(answererStampableId);
        }

        void _setResponseCode(const ResponseCode &code) {
            (*this)["r"] = (int)code;
        }

        void _setResponseData(const QVariant &data) {
            (*this)["rdata"] = data;
        }

};