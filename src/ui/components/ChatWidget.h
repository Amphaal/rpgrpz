#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>


#include "src/network/chat/client/ChatClient.h"

#include "ChatEdit.h"
#include "LogScrollView.h"


class ChatWidget : public QWidget {

    Q_OBJECT

    public:
        enum LogType { Default, ServerLog, ClientMessage };

        ChatWidget(QWidget *parent = nullptr);
        
        void createNewLog();
        void writeInChatLog(const std::string &message, ChatWidget::LogType logType = ChatWidget::LogType::Default);

        void bindToChatClient(ChatClient * cc);

    private:
        LogScrollView *_chatLog = 0;
        LogScrollView *_usersLog = 0;

        ChatEdit *_chatEdit = 0;

        QString serverName;

        ChatClient* _currentCC = 0;

        void _sendMessage();

        void _instUI();

        void _DisableUI();
        void _EnableUI();

        //
        void _onChatClientError(const std::string errMsg);
        void _onChatClientReceivedMessage(const std::string message);
        void _onChatClientReceivedHistory();
        void _onChatClientloggedUsersUpdated(QVariantList users);

};