#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

#include <QtGui/QWindow>
#include <QStyle>
#include "src/ui/components/others/RestoringSplitter.hpp"

#include <QGroupBox>

#include "src/network/rpz/client/RPZClient.h"

#include "ChatEdit.h"
#include "LogScrollView.h"


class ChatWidget : public QGroupBox {

    Q_OBJECT

    public:
        enum LogType { Default, ServerLog, ClientMessage };

        ChatWidget(QWidget *parent = nullptr);

    public slots: 
        void writeInChatLog(const std::string &message, const ChatWidget::LogType &logType = ChatWidget::LogType::Default);
        void bindToRPZClient(RPZClient * cc);

    private:
        LogScrollView *_chatLog = nullptr;
        LogScrollView *_usersLog = nullptr;
        ChatEdit *_chatEdit = nullptr;

        QString serverName;
        RPZClient* _currentCC = nullptr;

        void _sendMessage();

        //ui helpers
        void _instUI();

        //switches
        void _DisableUI();
        void _EnableUI();

    private slots:
        void _onRPZClientError(const std::string &errMsg);
        void _onReceivedMessage(const std::string &message);
        void _onReceivedLogHistory(const QVariantList &messages);
        void _onLoggedUsersUpdated(const QVariantHash &users);

};