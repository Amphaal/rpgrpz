#include "JSONSocket.h"

JSONSocket::JSONSocket(QThread* parentThread, const QString &logId, QTcpSocket* wrapped) : QObject(nullptr), _logId(logId) {

    if (wrapped) {
        this->_innerSocket = wrapped;
    } else {
        this->_innerSocket = new QTcpSocket(this);
    }

    this->_innerSocket->moveToThread(parentThread);

    QObject::connect(
        this->_innerSocket, &QIODevice::readyRead,
        this, &JSONSocket::_processIncomingData
    );

    QObject::connect(
        this->_innerSocket, &QIODevice::bytesWritten,
        this, &JSONSocket::_onBytesWritten
    );
}

void JSONSocket::_onBytesWritten() {
    qDebug() << this->_customLog("json sent !");
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

    qDebug() << this->_customLog("json to be sent >> " + JSONMethodAsArray[method]);

    //send !
    this->_sendJSONAsBinary(payload_doc.toBinaryData());
}


void JSONSocket::_sendJSONAsBinary(const QByteArray &data) {
    
    //send welcome message
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);

    //send...
    out << data;
    auto written = this->_innerSocket->write(block);

    this->_innerSocket->flush();

}

void JSONSocket::_processIncomingData() {
    
    //process incoming data
    QDataStream in(this->_innerSocket);
    in.setVersion(QDataStream::Qt_5_12);
    in.startTransaction();

    QByteArray block;
    in >> block;

    if (!in.commitTransaction()) {
        qWarning() << this->_customLog("issue while reading incoming data");  
        return;
    }

    this->_processIncomingAsJson(block);

}

void JSONSocket::_processIncomingAsJson(const QByteArray &data) {

    //parse to json
    auto json = QJsonDocument::fromBinaryData(data);
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

    auto method = static_cast<JSONMethod>((int)content["_m"].toDouble());
    emit JSONReceived(this, method, content["_d"].toVariant());

    qDebug() << this->_customLog("json received >> " + JSONMethodAsArray[method]);
}

QTcpSocket * JSONSocket::socket() {
    return this->_innerSocket;
}

QString JSONSocket::_customLog(const QString &text) {
    const QString temp = this->_logId + " : " + text;
    return temp;
}