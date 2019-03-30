#pragma once

#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

class JSONSocket : public QObject {

    Q_OBJECT

    public:
        JSONSocket(QString logId, QTcpSocket * wrapped = nullptr);
        void sendJSON(QString method, QVariant data);
        QTcpSocket* socket();
        
    signals:
        void JSONReceived(JSONSocket* wrapper, QString method, QVariant data);


    private:
        QString _logId;
        QDataStream * _inputBufferStream = nullptr;
        QTcpSocket * _innerSocket = nullptr;

        void _sendJSONAsBinary(QByteArray data);
        void _processIncomingData();
        void _processIncomingAsJson(QByteArray data);
        void _onBytesWritten();

        QString _customLog(QString text);
};