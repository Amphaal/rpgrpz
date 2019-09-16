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

#include "src/shared/atoms/AtomsStorage.h"
#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/models/RPZResponse.h"
#include "src/shared/models/RPZHandshake.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/helpers/_appContext.h"

#include "src/shared/commands/MessageInterpreter.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class RPZServer : public QTcpServer, public JSONRouter { 
    
    Q_OBJECT

    public:
        RPZServer();
        ~RPZServer();
    
    public slots:
        void run();

    signals:
        void listening();
        void error();
        void stopped();

    private:
        QHash<JSONSocket*, RPZUserId> _idsByClientSocket;
        JSONSocket* _hostSocket = nullptr;
        
        //music
        StreamPlayStateTracker _tracker;
        void _sendPlayedStream(JSONSocket* socket);

        //users
        RPZMap<RPZUser> _usersById;
        QHash<QString, RPZUser*> _formatedUsernamesByUser;
        RPZUser& _getUser(JSONSocket* socket);
        void _broadcastUsers();
        void _tellUserHisIdentity(JSONSocket* socket);

        //map atoms
        AtomsStorage* _hints = nullptr;
        void _askHostForMapHistory();
        void _broadcastMapChanges(AlterationPayload &payload, JSONSocket * senderSocket);
        void _sendMapHistory(JSONSocket * clientSocket);
            void _alterIncomingPayloadWithUpdatedOwners(AtomsWielderPayload &wPayload, JSONSocket * senderSocket);
        
        //messages
        RPZMap<RPZMessage> _messages;
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _interpretMessage(JSONSocket* sender, RPZMessage &msg);
        
        //internal
        void _onNewConnection();
        void _onClientSocketDisconnected(JSONSocket* disconnectedSocket);
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
        
        void _sendToAll(const JSONMethod &method, const QVariant &data);
        void _sendToAllButSelf(JSONSocket * senderSocket, const JSONMethod &method, const QVariant &data);
};