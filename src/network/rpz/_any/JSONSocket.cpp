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

void JSONSocket::sendJSON(const RPZJSON::Method &method, const QVariant &data) {

    emit sending();

    //ignore emission when socket is not connected
    if(this->socket()->state() != QAbstractSocket::ConnectedState) {
        qWarning() << qUtf8Printable(this->_logId) << ": cannot send JSON as the socket is not connected !";  
        emit sent();
        return;
    }

    //checks
    if(data.isNull()) {
        qWarning() << qUtf8Printable(this->_logId) << ": cannot send JSON as input values are unexpected";  
        emit sent();
        return;
    }

    //format document
    QJsonObject json_payload;
    json_payload.insert(QStringLiteral(u"_m"), (int)method);
    json_payload.insert(QStringLiteral(u"_d"), data.toJsonValue());
    QJsonDocument payload_doc(json_payload);

    //send !
    QDataStream out(this->_innerSocket);
    out.setVersion(QDataStream::Qt_5_13);

        //json to bytes
        auto bytes = qCompress(payload_doc.toJson(QJsonDocument::Compact));
        auto size = bytes.size();

        //write header
        out << (quint32)method;
        out << (quint32)size;

        //write data
        out << bytes;
    
    //log
    this->_debugLog(method, "sent");
    emit sent();
}

void JSONSocket::_debugLog(const QString &logId, const RPZJSON::Method &method, const QString &msg) {
    qDebug() << qUtf8Printable(logId) 
             << method
             << " : " 
             << qUtf8Printable(msg);
}

void JSONSocket::_debugLog(const RPZJSON::Method &method, const QString &msg) {
    JSONSocket::_debugLog(this->_logId, method, msg);
}

void JSONSocket::_processIncomingData() {
    
    //process incoming data
    QDataStream in(this->_innerSocket);
    in.setVersion(QDataStream::Qt_5_13);
    
    while(!in.atEnd()) {
        
        if(this->_batchComplete) {
            this->_batchComplete = false;
            this->_ackHeader = false;
        }

        //start transaction
        in.startTransaction();

        //determine method from first byte
        quint32 methodAsInt, jsonSize;
        in >> methodAsInt >> jsonSize;

        auto method = (RPZJSON::Method)methodAsInt;
        auto fullSize = jsonSize + 8;

        //tell that download started, prevent resend on same batch
        if(!this->_ackHeader) {
            this->_ackHeader = true;
            emit ackedBatch(method, fullSize);
        }

        //handle as transaction
        QByteArray block;
        in >> block;

        //break loop if batch is not complete
        this->_batchComplete = in.commitTransaction();
        if(!this->_batchComplete) {
            
            //update progress
            if(this->_ackHeader) {
                auto waitingBytes = this->_innerSocket->bytesAvailable();
                emit batchDownloading(method, waitingBytes);
            }

            //break the loop, effectively waiting for more bytes
            return;

        }
        
        //process batch
        emit batchDownloading(method, fullSize);
        this->_processIncomingAsJson(qUncompress(block));

    }

}

void JSONSocket::_processIncomingAsJson(const QByteArray &data) {

    //parse to json
    auto json = QJsonDocument::fromJson(data);
    if(json.isNull()) {
        qWarning() << qUtf8Printable(this->_logId) << ": Data received was not JSON and thus cannot be read.";
        return;
    }

    //prepare
    if(!json.isObject()) {
        qWarning() << qUtf8Printable(this->_logId) << ": JSON received is not Object and thus cannot be handled.";
        return;
    }

    auto content = json.object();
    auto mainKeys = content.keys();

    //check requirements
    auto required = mainKeys.contains(_methodKey) && mainKeys.contains(_dataKey);
    if(!required) {
        qWarning() << qUtf8Printable(this->_logId) << ": JSON received has missing keys and thus cannot be handled.";
        return;
    }

    //check value types
    auto rawMethod = content.value(_methodKey);
    if(!rawMethod.isDouble()) {
        qWarning() << qUtf8Printable(this->_logId) << ": JSON received has unexpected data and thus cannot be handled.";
        return;
    }

    //log
    auto method = (RPZJSON::Method)content.value(_methodKey).toInt();
    this->_debugLog(method, "received");

    //bind
    emit JSONReceived(
        this, 
        method, 
        content.value(_dataKey).toVariant()
    );

}

QTcpSocket * JSONSocket::socket() {
    return this->_innerSocket;
}