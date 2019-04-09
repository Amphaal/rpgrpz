#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/components/others/RestoringSplitter.hpp"

#include <QGroupBox>

#include "src/network/rpz/client/RPZClient.h"

#include "src/shared/RPZMessage.hpp"
#include "src/shared/RPZUser.hpp"
#include "src/shared/ClientBindable.hpp"

#include "ChatEdit.h"

#include "logs/UsersLog.hpp"
#include "logs/base/LogScrollView.h"



class ChatWidget : public QGroupBox, public ClientBindable {

    Q_OBJECT

    public:
        enum LogType { Default, ServerLog, ClientMessage };

        ChatWidget(QWidget *parent = nullptr);

    public slots: 
        void writeInChatLog(const std::string &message, const ChatWidget::LogType &logType = ChatWidget::LogType::Default);
        void bindToRPZClient(RPZClient* cc) override;

    private:
        LogScrollView *_chatLog;
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
        void _onRPZClientError(const std::string &errMsg);
        void _onReceivedMessage(const QVariantHash &message);
        void _onReceivedLogHistory(const QVariantList &messages);

};