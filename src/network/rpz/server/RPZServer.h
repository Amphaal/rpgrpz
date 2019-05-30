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
#include <QVector>
#include <QUuid>

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/map/MapHint.h"
#include "src/shared/models/Payloads.h"
#include "src/shared/models/entities/RPZMessage.hpp"
#include "src/shared/models/entities/RPZUser.hpp"

#include "src/shared/database/AssetsDatabase.h"

#include "src/helpers/_appContext.h"

class RPZServer : public QTcpServer, public JSONRouter { 
    
    Q_OBJECT

    public:
        RPZServer(QObject* parent = nullptr);
        ~RPZServer();
        void run();

    signals:
        void listening();
        void error();

    private:
        QHash<JSONSocket*, QUuid> _idsByClientSocket;
        JSONSocket* _hostSocket = nullptr;
        
        //users
        QHash<QUuid, RPZUser> _usersById;
        QVariantHash _serializeUsers();
        RPZUser* _getUser(JSONSocket* socket);
        void _broadcastUsers();
        void _tellUserHisIdentity(JSONSocket* socket);

        //map atoms
        MapHint* _hints;
        void _askHostForMapHistory();
        void _broadcastMapChanges(const QVariantHash &payload, JSONSocket * senderSocket);
        void _sendMapHistory(JSONSocket * clientSocket);
        
        //messages
        QHash<QUuid, RPZMessage> _messages;
        QVariantList _serializeMessages();
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _broadcastMessage(RPZMessage &messageToBroadcast);
        
        //internal
        void _onNewConnection();
        void _onDisconnect();
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
};