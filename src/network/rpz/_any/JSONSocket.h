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
        JSONSocket(QObject* parent, const QString &logId, QTcpSocket * socketToHandle = nullptr);
        ~JSONSocket();
        void sendJSON(const JSONMethod &method, const QVariant &data);
        QTcpSocket* socket();

        static void _debugLog(const QString &logId, const JSONMethod &method, const QString &msg);

    signals:
        void JSONReceived(JSONSocket* target, const JSONMethod &method, const QVariant &data);
        void ackedBatch(JSONMethod method, qint64 batchSize);
        void batchDownloading(JSONMethod method, qint64 downloaded);
        void sending();
        void sent();

    protected:
        void _debugLog(const JSONMethod &method, const QString &msg);

    private:
        bool _batchComplete = false;
        bool _ackHeader = false;

        QString _logId;
        bool _isWrapper = false;
        QTcpSocket* _innerSocket = nullptr;

        void _processIncomingData();
        void _processIncomingAsJson(const QByteArray &data);
};