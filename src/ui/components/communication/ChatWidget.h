#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/components/others/RestoringSplitter.h"

#include <QGroupBox>

#include "src/network/rpz/client/RPZClient.h"

#include "ChatEdit.h"
#include "LogScrollView.h"


class ChatWidget : public QGroupBox {

    Q_OBJECT

    public:
        enum LogType { Default, ServerLog, ClientMessage };

        ChatWidget(QWidget *parent = nullptr);
        
        void createNewLog();
        void writeInChatLog(const std::string &message, ChatWidget::LogType logType = ChatWidget::LogType::Default);

        void bindToRPZClient(RPZClient * cc);

    private:
        LogScrollView *_chatLog = 0;
        LogScrollView *_usersLog = 0;
        ChatEdit *_chatEdit = 0;

        QString serverName;
        RPZClient* _currentCC = 0;

        void _sendMessage();

        //ui helpers
        void _instUI();

        //switches
        void _DisableUI();
        void _EnableUI();

        //
        void _onRPZClientError(const std::string errMsg);
        void _onRPZClientReceivedMessage(const std::string message);
        void _onRPZClientReceivedHistory();
        void _onRPZClientloggedUsersUpdated(QVariantList users);

};