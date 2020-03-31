#include "JSONSocket.h"

JSONSocket::JSONSocket(QObject* parent, JSONLogger* logger, QTcpSocket* socketToHandle) : QObject(parent), _logger(logger) {

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

bool JSONSocket::sendToSocket(const RPZJSON::Method &method, const QVariant &data) {
    auto success =_sendToSocket(this, this->_logger, method, data);
    if(success) this->_logger->log(method, "sent");
    return success;
}

int JSONSocket::sendToSockets(
        JSONLogger* logger, 
        const QList<JSONSocket*> toSendTo, 
        const RPZJSON::Method &method, 
        const QVariant &data
    ) {
    
    auto expected = toSendTo.count();
    int sent = 0;

    for(const auto socket : toSendTo) {
        auto success = _sendToSocket(socket, logger, method, data);
        sent += (int)success;
    }

    logger->log(method, QStringLiteral(u"sent to [%1/%2] clients").arg(sent).arg(expected));
    return sent;
    
}

bool JSONSocket::_sendToSocket(JSONSocket* socket, JSONLogger* logger, const RPZJSON::Method &method, const QVariant &data) {

    emit socket->sending();

    //ignore emission when socket is not connected
    if(socket->socket()->state() != QAbstractSocket::ConnectedState) {
        logger->log("cannot send JSON as the socket is not connected");
        emit socket->sent(false);
        return false;
    }

    //checks
    if(data.isNull()) {
        logger->log("cannot send JSON as input values are unexpected");  
        emit socket->sent(false);
        return false;
    }

    //format document
    QJsonObject json_payload;
    json_payload.insert(QStringLiteral(u"_m"), (int)method);
    json_payload.insert(QStringLiteral(u"_d"), data.toJsonValue());
    QJsonDocument payload_doc(json_payload);

    //send !
    QDataStream out(socket->socket());
    out.setVersion(QDataStream::Qt_5_13);

        //json to bytes
        auto bytes = qCompress(payload_doc.toJson(QJsonDocument::Compact));
        auto size = bytes.size();

        //write header
        out << (quint32)method;
        out << (quint32)size;

        //write data
        out << bytes;

    logger->log(QStringLiteral("Sending %1...").arg(QLocale::system().formattedDataSize(size)));

    //ack success
    emit socket->sent(true);
    return true;
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
        this->_logger->log(QStringLiteral("Received %1...").arg(QLocale::system().formattedDataSize(fullSize)));

        this->_processIncomingAsJson(qUncompress(block));

    }

}

void JSONSocket::_processIncomingAsJson(const QByteArray &data) {

    //parse to json
    auto json = QJsonDocument::fromJson(data);
    if(json.isNull()) {
        this->_logger->log("Data received was not JSON and thus cannot be read");
        return;
    }

    //prepare
    if(!json.isObject()) {
        this->_logger->log("JSON received is not Object and thus cannot be handled");
        return;
    }

    auto content = json.object();
    auto mainKeys = content.keys();

    //check requirements
    auto required = mainKeys.contains(_methodKey) && mainKeys.contains(_dataKey);
    if(!required) {
        this->_logger->log("JSON received has missing keys and thus cannot be handled");
        return;
    }

    //check value types
    auto rawMethod = content.value(_methodKey);
    if(!rawMethod.isDouble()) {
        this->_logger->log("JSON received has unexpected data and thus cannot be handled");
        return;
    }

    //log
    auto method = (RPZJSON::Method)content.value(_methodKey).toInt();

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