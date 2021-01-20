// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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
#include <QQueue>
#include <QPair>

#include <cmath>

#include "RPZJSON.hpp"
#include "JSONLogger.hpp"

class JSONSocket : public QTcpSocket {
    Q_OBJECT

 public:
    JSONSocket(QObject* parent, JSONLogger* logger);
    ~JSONSocket();

    bool sendToSocket(const RPZJSON::Method &method, const QVariant &data);

 signals:
    void PayloadReceived(const RPZJSON::Method &method, const QVariant &data);

    void JSONReceivingStarted(RPZJSON::Method method, qint64 totalToDownload);
    void JSONDownloading(qint64 totalBytesDownloadedForBatch);
    void JSONDownloaded();

    void JSONSendingFailed();
    void JSONSendingStarted(RPZJSON::Method method, qint64 totalToUpload);
    void JSONUploading(qint64 bytesUploaded);
    void JSONUploadInterrupted();
    void JSONUploaded();

 private:
    static inline const int _maxUploadChunkSize = 64 * 1024;  // 64 KB
    bool _batchComplete = false;
    bool _ackHeader = false;

    JSONLogger* _logger = nullptr;

    void _processIncomingData();
    void _processIncomingAsJson(const QByteArray &data);

    bool _send(const RPZJSON::Method &method, const QVariant &data);
    void _onBytesWritten(qint64 bytes);
    QQueue<QPair<RPZJSON::Method, qint64>> _waitingTBU;

    static inline QString _dataKey = QStringLiteral(u"_d");
    static inline QString _methodKey = QStringLiteral(u"_m");
};
