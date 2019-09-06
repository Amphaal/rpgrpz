#include "JSONSocket.h"

JSONSocket::JSONSocket(QObject* parent, const QString &logId, QTcpSocket* socketToHandle) : QObject(parent), _logId(logId) {

    if (socketToHandle) {
        this->_isWrapper = true;
        this->_innerSocket = socketToHandle;
    } else {
        this->_innerSocket = new QTcpSocket;
    }

    QObject::connect(
        this->_innerSocket, &QIODevice::readyRead,
        this, &JSONSocket::_processIncomingData
    );

}

JSONSocket::~JSONSocket() {
    if(!this->_isWrapper) {
        this->_innerSocket->close();
        delete this->_innerSocket;
    }
}

void JSONSocket::sendJSON(const JSONMethod &method, const QVariant &data) {
    
    //ignore emission when socket is not connected
    if(this->socket()->state() != QAbstractSocket::ConnectedState) {
        qWarning() << this->_logId.toStdString().c_str() << ": cannot send JSON as the socket is not connected !";  
        return;
    }

    //checks
    if(data.isNull()) {
        qWarning() << this->_logId.toStdString().c_str() << ": cannot send JSON as input values are unexpected";  
        return;
    }

    //format document
    QJsonObject json_payload;
    json_payload["_m"] = method;
    json_payload["_d"] = data.toJsonValue();
    QJsonDocument payload_doc(json_payload);

    //send !
    QDataStream out(this->_innerSocket);
    out.setVersion(QDataStream::Qt_5_13);
    out << payload_doc.toJson(QJsonDocument::Compact);
    
    //log
    this->_debugLog(method, "sent");
}

void JSONSocket::_debugLog(const QString &logId, const JSONMethod &method, const QString &msg) {
    qDebug() << logId.toStdString().c_str() 
             << QString("[" + JSONMethodAsArray[method] + "]").toStdString().c_str() 
             << ":" << msg.toStdString().c_str();
}

void JSONSocket::_debugLog(const JSONMethod &method, const QString &msg) {
    JSONSocket::_debugLog(this->_logId, method, msg);
}

void JSONSocket::_processIncomingData() {
    
    //process incoming data
    QByteArray block;
    QDataStream in(this->_innerSocket);
    in.setVersion(QDataStream::Qt_5_13);
    
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
        qWarning() << this->_logId.toStdString().c_str() << ": Data received was not JSON and thus cannot be read.";
        return;
    }

    //prepare
    if(!json.isObject()) {
        qWarning() << this->_logId.toStdString().c_str() << ": JSON received is not Object and thus cannot be handled.";
        return;
    }

    auto content = json.object();
    auto mainKeys = content.keys();

    //check requirements
    auto required = mainKeys.contains("_m") && mainKeys.contains("_d");
    if(!required) {
        qWarning() << this->_logId.toStdString().c_str() << ": JSON received has missing keys and thus cannot be handled.";
        return;
    }

    //check value types
    auto okTypes = content["_m"].isDouble();
    if(!okTypes) {
        qWarning() << this->_logId.toStdString().c_str() << ": JSON received has unexpected data and thus cannot be handled.";
        return;
    }

    //log
    auto method = static_cast<JSONMethod>((int)content["_m"].toDouble());
    this->_debugLog(method, "received");

    //bind
    emit JSONReceived(this, method, content["_d"].toVariant());
}

QTcpSocket * JSONSocket::socket() {
    return this->_innerSocket;
}