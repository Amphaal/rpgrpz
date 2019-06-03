#include "JSONSocket.h"

JSONSocket::JSONSocket(QObject* parent, const QString &logId, QTcpSocket* wrapped) : QObject(parent), _logId(logId) {

    if (wrapped) {
        this->_innerSocket = wrapped;
    } else {
        this->_innerSocket = new QTcpSocket(this);
    }

    QObject::connect(
        this->_innerSocket, &QIODevice::readyRead,
        this, &JSONSocket::_processIncomingData
    );

    //clear on client disconnect
    QObject::connect(
        this->_innerSocket, &QAbstractSocket::disconnected,
        [&]() {
            emit disconnected();
        }
    );
}

void JSONSocket::sendJSON(const JSONMethod &method, const QVariant &data) {
    
    //checks
    if(data.isNull()) {
        qWarning() << this->_customLog("cannot send JSON as input values are unexpected");  
        return;
    }

    //format document
    QJsonObject json_payload;
    json_payload["_m"] = method;
    json_payload["_d"] = data.toJsonValue();
    QJsonDocument payload_doc(json_payload);

    //send !
    QDataStream out(this->_innerSocket);
    out.setVersion(QDataStream::Qt_5_12);
    out << payload_doc.toJson(QJsonDocument::Compact);
    
}

void JSONSocket::_processIncomingData() {
    
    //process incoming data
    QByteArray block;
    QDataStream in(this->_innerSocket);
    in.setVersion(QDataStream::Qt_5_12);
    
    for (;;) {

        in.startTransaction();

        in >> block;

        if (in.commitTransaction()) {
            
            this->_processIncomingAsJson(block);

        } else {
            // the read failed, the socket goes automatically back to the state it was in before the transaction started
            // we just exit the loop and wait for more data to become available
            break;
        }
    }

}

void JSONSocket::_processIncomingAsJson(const QByteArray &data) {

    //parse to json
    auto json = QJsonDocument::fromJson(data);
    if(json.isNull()) {
        qWarning() << this->_customLog("Data received was not JSON and thus cannot be read.");
        return;
    }

    //prepare
    if(!json.isObject()) {
        qWarning() << this->_customLog("JSON received is not Object and thus cannot be handled.");
        return;
    }

    auto content = json.object();
    auto mainKeys = content.keys();

    //check requirements
    auto required = mainKeys.contains("_m") && mainKeys.contains("_d");
    if(!required) {
        qWarning() << this->_customLog("JSON received has missing keys and thus cannot be handled.");
        return;
    }

    //check value types
    auto okTypes = content["_m"].isDouble();
    if(!okTypes) {
        qWarning() << this->_customLog("JSON received has unexpected data and thus cannot be handled.");
        return;
    }

    //signal
    auto method = static_cast<JSONMethod>((int)content["_m"].toDouble());
    
    //bind
    emit JSONReceived(this, method, content["_d"].toVariant());
}

QTcpSocket * JSONSocket::socket() {
    return this->_innerSocket;
}

QString JSONSocket::_customLog(const QString &text) {
    const QString temp = this->_logId + " : " + text;
    return temp;
}