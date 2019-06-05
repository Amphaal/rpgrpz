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

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/_any/JSONRouter.h"

#include "src/shared/map/MapHint.h"
#include "src/shared/models/Payloads.h"
#include "src/shared/models/entities/RPZMessage.hpp"
#include "src/shared/models/entities/RPZUser.hpp"
#include "src/shared/models/entities/RPZResponse.hpp"
#include "src/shared/models/entities/RPZHandshake.hpp"

#include "src/shared/database/AssetsDatabase.h"

#include "src/helpers/_appContext.h"

#include "src/shared/command/MessageInterpreter.hpp"

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
        QHash<JSONSocket*, snowflake_uid> _idsByClientSocket;
        JSONSocket* _hostSocket = nullptr;
        
        //users
        RPZMap<RPZUser> _usersById;
        QHash<QString, RPZUser*> _formatedUsernamesByUser;
        RPZUser* _getUser(JSONSocket* socket);
        void _broadcastUsers();
        void _tellUserHisIdentity(JSONSocket* socket);

        //map atoms
        MapHint* _hints;
        void _askHostForMapHistory();
        void _broadcastMapChanges(QVariantHash &payload, JSONSocket * senderSocket);
        void _sendMapHistory(JSONSocket * clientSocket);
        AlterationPayload _alterIncomingPayloadWithUpdatedOwners(QVariantHash &payload, JSONSocket * senderSocket);
        
        //messages
        RPZMap<RPZMessage> _messages;
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _interpretMessage(JSONSocket* sender, RPZMessage &msg);
        
        //internal
        void _onNewConnection();
        void _onDisconnect();
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
        
        void _sendToAll(const JSONMethod &method, const QVariant &data);
        void _sendToAllButSelf(JSONSocket * senderSocket, const JSONMethod &method, const QVariant &data);
};