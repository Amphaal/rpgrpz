#pragma once

#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

#include "JSONMethod.h"

class JSONSocket : public QObject {

    Q_OBJECT

    public:
        JSONSocket(const QString &logId, QTcpSocket * wrapped = nullptr);
        void sendJSON(const JSONMethod &method, const QVariant &data);
        QTcpSocket* socket();

    signals:
        void JSONReceived(JSONSocket* target, const JSONMethod &method, const QVariant &data);

    private:
        QString _logId;
        QDataStream * _inputBufferStream = nullptr;
        QTcpSocket * _innerSocket = nullptr;

        void _sendJSONAsBinary(const QByteArray &data);
        void _processIncomingData();
        void _processIncomingAsJson(const QByteArray &data);
        void _onBytesWritten();

        QString _customLog(const QString &text);
};