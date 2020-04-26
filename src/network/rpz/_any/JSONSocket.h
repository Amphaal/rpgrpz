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

#pragma once

#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>

#include "RPZJSON.hpp"
#include "JSONLogger.hpp"

class JSONSocket : public QTcpSocket {
    Q_OBJECT

 public:
    JSONSocket(QObject* parent, JSONLogger* logger);

    bool sendToSocket(const RPZJSON::Method &method, const QVariant &data);
    static int sendToSockets(
        JSONLogger* logger,
        const QList<JSONSocket*> toSendTo,
        const RPZJSON::Method &method,
        const QVariant &data
    );

 signals:
    void PayloadReceived(const RPZJSON::Method &method, const QVariant &data);

    void JSONReceivingStarted(RPZJSON::Method method, qint64 totalToDownload);
    void JSONDownloading(qint64 batchDownloaded);
    void JSONDownloaded();

    void JSONSendingFailed();
    void JSONSendingStarted(RPZJSON::Method method, qint64 totalToUpload);
    void JSONUploading(qint64 batchUploaded);
    void JSONUploaded();

 private:
    bool _batchComplete = false;
    bool _ackHeader = false;

    JSONLogger* _logger = nullptr;

    void _processIncomingData();
    void _processIncomingAsJson(const QByteArray &data);
    void _onBytesWritten(qint64 bytes);

    static bool _sendToSocket(JSONSocket* socket, JSONLogger* logger, const RPZJSON::Method &method, const QVariant &data);

    static inline QString _dataKey = QStringLiteral(u"_d");
    static inline QString _methodKey = QStringLiteral(u"_m");
};
