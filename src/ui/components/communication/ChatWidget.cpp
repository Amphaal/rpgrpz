#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : 
            QGroupBox(parent),
            _chatLog(new LogScrollView),
            _usersLog(new UsersLog),
            _chatEdit(new ChatEdit) {

        //this...
        this->_DisableUI();

        //UI...
        this->_instUI();
    
}

void ChatWidget::_instUI() {

    this->setTitle("Chat de la partie");
    this->setAlignment(Qt::AlignHCenter);
    this->setLayout(new QVBoxLayout);
    
    ///////////////
    // left part //
    ///////////////

    auto _left = [&]() {
        auto left = new QWidget;
        left->setLayout(new QVBoxLayout);
        left->layout()->setMargin(0);
        
        //chat area...
        left->layout()->addWidget(this->_chatLog);

        //messaging...
        left->layout()->addWidget(this->_chatEdit);
        
        return left;
    };
    
    ////////////////
    // right part //
    ////////////////

    auto _right = [&]() {
        this->_usersLog->setMinimumWidth(100);
        return this->_usersLog;
    };

    ////////////
    // Fusion //
    ////////////
    auto splitter = new RestoringSplitter("ChatWidgetSplitter");
    splitter->addWidget(_left());
    splitter->addWidget(_right());
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->loadState();
    this->layout()->addWidget(splitter);
}

void ChatWidget::_onRPZClientError(const std::string &errMsg) {    
    
    //out log
    if(!this->serverName.isEmpty()) {
        const auto nm = errMsg + " (" + this->serverName.toStdString() + ")";
        this->writeInChatLog(nm, ChatWidget::LogType::ServerLog);
    }

    this->_DisableUI();

}
void ChatWidget::_onReceivedMessage(const QVariantHash &message) {
    auto msg = RPZMessage::fromVariantHash(message);
    auto str_msg = msg.toString().toStdString();
    
    //write in log
    this->writeInChatLog(str_msg);

}

void ChatWidget::_onReceivedLogHistory(const QVariantList &messages) {

    //add list of messages
    for(auto &msg : messages) {
        this->_onReceivedMessage(msg.toHash());
    }

    //welcome msg
    const auto welcome = QString("Connecté au serveur (") + this->serverName + ")";
    this->writeInChatLog(welcome.toStdString(), ChatWidget::LogType::ServerLog);
}

void ChatWidget::bindToRPZClient(RPZClient * cc) {

    ClientBindable::bindToRPZClient(cc);
    
    this->serverName = cc->getConnectedSocketAddress();
    this->_usersLog->newLog();
    this->_chatLog->newLog();

    //on error from client
    QObject::connect(
        this->_rpzClient, &RPZClient::error, 
        this, &ChatWidget::_onRPZClientError
    );
    
    //on message received
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedMessage, 
        this, &ChatWidget::_onReceivedMessage
    );

    //welcome once all history have been received
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedLogHistory, 
        this, &ChatWidget::_onReceivedLogHistory
    );

    //update users list
    QObject::connect(
        this->_rpzClient, &RPZClient::loggedUsersUpdated,
        this->_usersLog, &UsersLog::updateUsers
    );

    //enable UI at connection
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedLogHistory, 
        this, &ChatWidget::_EnableUI
    );

    //on message typed 
    QObject::connect(
        this->_chatEdit, &ChatEdit::askedToSendMessage,
        this->_rpzClient, &RPZClient::sendMessage
    );

}

void ChatWidget::_DisableUI() {
    this->_chatEdit->setEnabled(false);
    this->_usersLog->setEnabled(false);
    this->_chatLog->setEnabled(false);
}

void ChatWidget::_EnableUI() {
    this->_chatEdit->setEnabled(true);
    this->_usersLog->setEnabled(true);
    this->_chatLog->setEnabled(true);
}


void ChatWidget::writeInChatLog(const std::string &message, const ChatWidget::LogType &logType) {
    
    QPalette* colors = nullptr;

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
