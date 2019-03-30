#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : 
            QWidget(parent),
            _chatLog(new LogScrollView),
            _usersLog(new LogScrollView),
            _chatEdit(new ChatEdit) {
        
        QObject::connect(
            this->_chatEdit, &ChatEdit::askedToSendMessage,
            [&](QString msg) {
                this->_currentCC->sendMessage(msg);
            }
        );

        //this...
        this->_DisableUI();

        //UI...
        this->_instUI();
    
}

void ChatWidget::_instUI() {

    this->setLayout(new QHBoxLayout);
    this->layout()->setContentsMargins(10, 0, 10, 0);
    
    ///////////////
    // left part //
    ///////////////

    auto left = new QWidget(this);
    left->setLayout(new QVBoxLayout);
    left->layout()->setContentsMargins(0, 0, 0, 0);
    
    //chat area...
    left->layout()->addWidget(this->_chatLog);

    //messaging...
    left->layout()->addWidget(this->_chatEdit);
    this->layout()->addWidget(left);
    
    ////////////////
    // right part //
    ////////////////

    this->_usersLog->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    this->layout()->addWidget(this->_usersLog);
}


void ChatWidget::_onChatClientError(const std::string errMsg) {    
    
    //out log
    if(!this->serverName.isEmpty()) {
        auto nm = errMsg + " (" + this->serverName.toStdString() + ")";
        this->writeInChatLog(nm, ChatWidget::LogType::ServerLog);
    }

    this->_DisableUI();

}
void ChatWidget::_onChatClientReceivedMessage(const std::string message) {
    this->writeInChatLog(message);
}

void ChatWidget::_onChatClientReceivedHistory() {
    auto msg = QString("Connecté au serveur (") + this->serverName + ")";
    this->writeInChatLog(msg.toStdString(), ChatWidget::LogType::ServerLog);
}

void ChatWidget::bindToChatClient(ChatClient * cc) {

    this->_currentCC = cc;

    //initial message to log
    auto socketAddr = this->_currentCC->getConnectedSocketAddress();

    //on error from client
    QObject::connect(
        this->_currentCC, &ChatClient::error, 
        this, &ChatWidget::_onChatClientError
    );
    
    //on message received
    QObject::connect(
        this->_currentCC, &ChatClient::receivedMessage, 
        this, &ChatWidget::_onChatClientReceivedMessage
    );

    //welcome once all history have been received
    QObject::connect(
        this->_currentCC, &ChatClient::historyReceived, 
        this, &ChatWidget::_onChatClientReceivedHistory
    );

    //enable UI at connection
    QObject::connect(
        this->_currentCC, &ChatClient::connected, 
        this, &ChatWidget::_EnableUI
    );

    QObject::connect(
        this->_currentCC, &QObject::destroyed,
        [&]() {
            this->_currentCC = 0;
        }
    );

}

void ChatWidget::_DisableUI() {
    this->serverName = "";
    this->_chatEdit->setEnabled(false);
    this->setEnabled(false);
}

void ChatWidget::_EnableUI(QString serverAddress) {
    this->serverName = serverAddress;
    this->_chatEdit->setEnabled(true);
    this->setEnabled(true);
    this->_usersLog->newLog();
    this->_chatLog->newLog();
}


void ChatWidget::writeInChatLog(const std::string &message, ChatWidget::LogType logType) {
    
    QPalette* colors = 0;

    switch(logType) {
        case ChatWidget::LogType::ServerLog:
            colors = new QPalette();
            colors->setColor(QPalette::Window, "#71ed55");
            colors->setColor(QPalette::WindowText, Qt::black);
            break;
        case ChatWidget::LogType::ClientMessage:
            break;
    }

    this->_chatLog->writeAtEnd(message, colors);
};
