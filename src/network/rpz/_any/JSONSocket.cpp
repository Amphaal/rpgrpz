// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "JSONSocket.h"

JSONSocket::JSONSocket(QObject* parent, JSONLogger* logger) : QTcpSocket(parent), _logger(logger) {
    QObject::connect(
        this, &QIODevice::readyRead,
        this, &JSONSocket::_processIncomingData
    );
    QObject::connect(
        this, &QIODevice::bytesWritten,
        this, &JSONSocket::_onBytesWritten
    );
}

JSONSocket::~JSONSocket() {
    this->disconnectFromHost();  // force disconnection before destruction for event handling
}

bool JSONSocket::sendToSocket(const RPZJSON::Method &method, const QVariant &data) {
    auto success = this->_send(method, data);
    if (success) this->_logger->log(method, ">>");
    return success;
}

void JSONSocket::_onBytesWritten(qint64 bytes) {
    while(bytes) {
        auto &wtbu = this->_waitingTBU.head();
        emit JSONUploading(bytes);
        if (bytes >= wtbu.second) {
            emit JSONUploaded();
            bytes -= wtbu.second;
            this->_waitingTBU.dequeue();
        } else {
            bytes = 0;
        }
    }
}

bool JSONSocket::_send(const RPZJSON::Method &method, const QVariant &data) {
    // ignore emission when socket is not connected
    if (auto state = this->state(); state != QAbstractSocket::ConnectedState) {
        this->_logger->log("cannot send JSON as the socket is not connected");
        emit JSONSendingFailed();
        return false;
    }

    // checks
    if (data.isNull()) {
        this->_logger->log("cannot send JSON as input values are unexpected");
        emit JSONSendingFailed();
        return false;
    }

    // format document
    QJsonObject json_payload;
    json_payload.insert(QStringLiteral(u"_m"), (int)method);
    json_payload.insert(QStringLiteral(u"_d"), data.toJsonValue());
    QJsonDocument payload_doc(json_payload);

    // json to bytes
    auto bytes = qCompress(payload_doc.toJson(QJsonDocument::Compact));
    auto size = bytes.size();

    auto beforeBTW = this->bytesToWrite();

    // send !
    QDataStream out(this);
    out.setVersion(QDataStream::Qt_5_13);

        // write header
        out << (quint32)method;
        out << (quint32)size;

        // write data
        out << bytes;

    auto afterBTW = this->bytesToWrite();
    auto payloadTotalSize = afterBTW - beforeBTW;

    // tell that sending has begun
    emit JSONSendingStarted(method, payloadTotalSize);
    this->_waitingTBU.enqueue({method, payloadTotalSize});

    return true;
}

void JSONSocket::_processIncomingData() {
    // process incoming data
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_13);

    while (!in.atEnd()) {
        if (this->_batchComplete) {
            this->_batchComplete = false;
            this->_ackHeader = false;
        }

        // start transaction
        in.startTransaction();

        // determine method from first byte
        quint32 methodAsInt, jsonSize;
        in >> methodAsInt >> jsonSize;

        // tell that download started, prevent resend on same batch
        if (!this->_ackHeader) {
            auto method = (RPZJSON::Method)methodAsInt;
            auto fullSize = jsonSize;
            this->_ackHeader = true;
            emit JSONReceivingStarted(method, fullSize);
        }

        // handle as transaction
        QByteArray block;
        in >> block;

        // break loop if batch is not complete
        this->_batchComplete = in.commitTransaction();
        if (!this->_batchComplete) {
            // update progress
            if (this->_ackHeader) {
                auto waitingBytes = this->bytesAvailable();
                emit JSONDownloading(waitingBytes);
            }

            // break the loop, effectively waiting for more bytes
            return;
        }

        // log
        auto compressedSize = block.count();
        emit JSONDownloading(compressedSize);
        emit JSONDownloaded();

        // payload is complete, decompress it
        auto decompressed = qUncompress(block);

        // process batch
        this->_processIncomingAsJson(decompressed);
    }
}

void JSONSocket::_processIncomingAsJson(const QByteArray &data) {
    // parse to json
    auto json = QJsonDocument::fromJson(data);
    if (json.isNull()) {
        this->_logger->log("Data received was not JSON and thus cannot be read");
        return;
    }

    // prepare
    if (!json.isObject()) {
        this->_logger->log("JSON received is not Object and thus cannot be handled");
        return;
    }

    auto content = json.object();
    auto mainKeys = content.keys();

    // check requirements
    auto required = mainKeys.contains(_methodKey) && mainKeys.contains(_dataKey);
    if (!required) {
        this->_logger->log("JSON received has missing keys and thus cannot be handled");
        return;
    }

    // check value types
    auto rawMethod = content.value(_methodKey);
    if (!rawMethod.isDouble()) {
        this->_logger->log("JSON received has unexpected data and thus cannot be handled");
        return;
    }

    // log
    auto method = (RPZJSON::Method)content.value(_methodKey).toInt();

    this->_logger->log(method, "<<");

    // bind
    emit PayloadReceived(method, content.value(_dataKey).toVariant());
}
