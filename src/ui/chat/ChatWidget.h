#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/_others/RestoringSplitter.h"

#include "src/network/rpz/client/RPZClient.h"

#include "src/shared/models/RPZMessage.h"
#include "src/shared/models/RPZResponse.h"

#include "src/shared/models/RPZUser.h"
#include "src/ui/_others/ClientBindable.h"

#include "ChatEdit.h"

#include "logs/base/LogScrollView.h"
#include "logs/MessagesLog.h"


class ChatWidget : public QWidget, public ClientBindable {

    public:
        ChatWidget(QWidget *parent = nullptr);

        void onRPZClientConnecting() override;

    private:
        MessagesLog *_chatLog;
        ChatEdit *_chatEdit;

        QString serverName;

        void _sendMessage();

        //ui helpers
        void _instUI();

        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onReceivedLogHistory(const QVector<RPZMessage> &messages);

};