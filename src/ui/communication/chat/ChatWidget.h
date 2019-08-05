#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/others/RestoringSplitter.h"

#include "src/network/rpz/client/RPZClientThread.h"

#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZResponse.h"

#include "src/shared/models/RPZUser.h"
#include "src/ui/others/ClientBindable.h"

#include "ChatEdit.h"

#include "logs/base/LogScrollView.h"
#include "logs/UsersLog.h"
#include "logs/MessagesLog.h"


class ChatWidget : public QWidget, public ClientBindable {

    public:
        ChatWidget(QWidget *parent = nullptr);

        void onRPZClientThreadConnecting() override;

    private:
        MessagesLog *_chatLog;
        UsersLog *_usersLog;
        ChatEdit *_chatEdit;

        QString serverName;

        void _sendMessage();

        //ui helpers
        void _instUI();

        //switches
        void _DisableUI();
        void _EnableUI();

        void _onRPZClientThreadStatus(const QString &statusMsg, bool isError);
        void _onReceivedLogHistory(const QVariantList &messages);

};