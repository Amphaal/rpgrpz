#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/others/RestoringSplitter.h"

#include "src/network/rpz/client/RPZClient.h"

#include "src/shared/models/RPZMessage.hpp"
#include "src/shared/models/RPZResponse.hpp"

#include "src/shared/models/RPZUser.hpp"
#include "src/ui/others/ClientBindable.h"

#include "ChatEdit.h"

#include "logs/base/LogScrollView.h"
#include "logs/UsersLog.hpp"
#include "logs/MessagesLog.hpp"


class ChatWidget : public QWidget, public ClientBindable {

    Q_OBJECT

    public:
        ChatWidget(QWidget *parent = nullptr);

    public slots: 
        void onRPZClientConnecting(RPZClient* cc) override;

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

    private slots:
        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onReceivedLogHistory(const QVariantList &messages);

};