#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/_others/RestoringSplitter.h"

#include "src/network/rpz/client/RPZClient.h"

#include "src/shared/models/messaging/RPZMessage.h"
#include "src/shared/models/messaging/RPZResponse.h"

#include "src/shared/models/RPZUser.h"
#include "src/ui/_others/ConnectivityObserver.h"

#include "ChatEdit.h"

#include "src/ui/chat/logs/_base/LogScrollView.h"
#include "src/ui/chat/logs/MessagesLog.h"


class ChatWidget : public QWidget, public ConnectivityObserver {

    public:
        ChatWidget(QWidget *parent = nullptr);

        void connectingToServer() override;

    private:
        MessagesLog *_chatLog;
        ChatEdit *_chatEdit;

        QString serverName;

        void _sendMessage();

        //ui helpers
        void _instUI();

        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onGameSessionReceived(const RPZGameSession &gameSession);

};