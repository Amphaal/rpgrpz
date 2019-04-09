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

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/MapHint.h"

#include "src/shared/RPZMessage.hpp"
#include "src/shared/RPZUser.hpp"

#include "src/helpers/_const.hpp"


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

        //map assets
        MapHint* _hints;
        void _askHostForMapHistory();
        void _broadcastMapChanges(const QVariantList &changes);
        
        //messages
        QHash<QUuid, RPZMessage> _messages;
        QVariantList _serializeMessages();
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _broadcastMessage(RPZMessage &messageToBroadcast);
        

        void _onNewConnection();
        void _onDisconnect();

        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
};