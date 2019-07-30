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

        static void _debugLog(const QString &logId, const JSONMethod &method, const QString &msg);

    signals:
        void JSONReceived(JSONSocket* target, const JSONMethod &method, const QVariant &data);
        void disconnected();

    protected:
        void _debugLog(const JSONMethod &method, const QString &msg);

    private:
        QString _logId;
        QTcpSocket * _innerSocket = nullptr;

        void _processIncomingData();
        void _processIncomingAsJson(const QByteArray &data);
};