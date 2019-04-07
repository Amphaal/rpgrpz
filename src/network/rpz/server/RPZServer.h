#pragma once

#include <QDebug>
#include <QTcpServer>
#include <QHostAddress>
#include <QVector>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QUuid>

#include "RPZSThread.h"
#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/MapHint.h"

#include "src/helpers/_const.hpp"

class RPZServer : public RPZSThread, public JSONRouter { 
    
    Q_OBJECT

    public:
        RPZServer();
        void run() override;

    private:
        QHash<JSONSocket*, QUuid> _idsByClientSocket;
        QHash<QUuid, JSONSocket*> _clientSocketsById;
        QHash<QUuid, QString> _clientDisplayNames;
        JSONSocket* _hostSocket = nullptr;
        QTcpServer* _server;

        //map assets
        MapHint* _hints;
        void _askHostForMapHistory();
        void _broadcastMapChanges(const QVariantList &changes);
        
        //messages
        QStringList _messages;
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _broadcastMessage(const QString &messageToBroadcast);
        void _broadcastUsers();

        void _onNewConnection();
        QString _getSocketDisplayName(JSONSocket * clientSocket);

        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
};