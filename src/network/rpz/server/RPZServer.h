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

#include "src/shared/hints/AtomsStorage.h"
#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZUser.h"
#include "src/shared/models/RPZResponse.h"
#include "src/shared/models/RPZHandshake.h"

#include "src/shared/database/AssetsDatabase.h"

#include "src/helpers/_appContext.h"
#include "src/helpers/StringHelper.hpp"

#include "src/shared/commands/MessageInterpreter.h"
#include "src/shared/audio/StreamPlayStateTracker.hpp"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"
#include "src/helpers/StringHelper.hpp"

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
        RPZMap<RPZUser> _usersById;
        QHash<JSONSocket*, RPZUserId> _idsByClientSocket;
        QHash<RPZUserId, JSONSocket*> _clientSocketById;
        QHash<QString, RPZUserId> _formatedUsernamesByUserId;
        JSONSocket* _hostSocket = nullptr;
        
        //music
        StreamPlayStateTracker _tracker;
        void _sendPlayedStream(JSONSocket* socket);

        //users
        RPZUser& _getUser(JSONSocket* socket);
        JSONSocket* _getUserSocket(const QString &formatedUsername);
        void _newUserAcknoledged(JSONSocket* socket, const RPZUser &userToAck);
        void _attributeRoleToUser(JSONSocket* socket, RPZUser &associatedUser, const RPZHandshake &handshake);

        //map atoms
        AtomsStorage* _hints = nullptr;
        void _broadcastMapChanges(JSONMethod method, AlterationPayload &payload, JSONSocket * senderSocket);
        void _sendMapHistory(JSONSocket * clientSocket);
            void _alterIncomingPayloadWithUpdatedOwners(AtomsWielderPayload &wPayload, JSONSocket * senderSocket);
        
        //messages
        RPZMap<RPZMessage> _messages;
        void _sendStoredMessages(JSONSocket * clientSocket);
        void _interpretMessage(JSONSocket* sender, RPZMessage &msg);
        void _maySendAndStoreDiceThrows(const QString &text);
        
        //internal
        void _onNewConnection();
        void _onClientSocketDisconnected(JSONSocket* disconnectedSocket);
        void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) override;
        
        void _sendToAll(const JSONMethod &method, const QVariant &data);
        void _sendToAllButSelf(JSONSocket* toExclude, const JSONMethod &method, const QVariant &data);
};