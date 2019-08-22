#include "RPZResponse.h"

RPZResponse::RPZResponse() : Stampable() {}
RPZResponse::RPZResponse(snowflake_uid answererTo, const ResponseCode &code, const QVariant &data) : Stampable() {
    this->_setResponseCode(code);
    if(answererTo) this->_setAnswerer(answererTo);
    if(!data.isNull()) this->_setResponseData(data);
}
RPZResponse::RPZResponse(const QVariantHash &hash) : Stampable(hash) {}

RPZResponse::ResponseCode RPZResponse::responseCode() const {
    return (ResponseCode)this->value("r").toInt();
}

QVariant RPZResponse::responseData() const {
    return this->value("rdata");
}

snowflake_uid RPZResponse::answerer() const {
    return this->value("aswr").toULongLong();
}

QString RPZResponse::toString() const{
    
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

QPalette RPZResponse::palette() const {
    
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

        default:
            break;
    }

    return palette;
}

void RPZResponse::_setAnswerer(snowflake_uid answererStampableId) {
    this->insert("aswr", QString::number(answererStampableId));
}

void RPZResponse::_setResponseCode(const ResponseCode &code) {
    this->insert("r", (int)code);
}

void RPZResponse::_setResponseData(const QVariant &data) {
    this->insert("rdata", data);
}