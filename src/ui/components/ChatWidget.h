#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QScrollArea>
#include <QtGui/QWindow>
#include <QStyle>
#include <QLineEdit>

#include "src/network/chat/client/ChatClient.h"

#include "LogScrollView.h"

class ChatWidget : public QWidget {

    Q_OBJECT

    public:
        enum LogType { Default, ServerLog, ClientMessage };

        ChatWidget(QWidget *parent = nullptr);
        
        void createNewLog();
        void printLog(const std::string &message, ChatWidget::LogType logType = ChatWidget::LogType::Default);

        void bindToChatClient(ChatClient * cc);

    private:
        QPushButton *sendBtn = 0;
        LogScrollView *lsv = 0;
        QScrollArea *scrollArea = 0;
        QLineEdit* msgEdit = 0;
        QString serverName;

        ChatClient* _currentCC = 0;

        void _sendMessage();

        void _scrollUpdate();
        void _instUI();

        void _DisableUI();
        void _EnableUI(QString serverAddress);

        //
        void _onChatClientError(const std::string errMsg);
        void _onChatClientReceivedMessage(const std::string message);
        void _onChatClientReceivedHistory();

};