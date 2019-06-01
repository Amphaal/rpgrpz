#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : 
            QWidget(parent),
            _chatLog(new MessagesLog(this)),
            _usersLog(new UsersLog(this)),
            _chatEdit(new ChatEdit(this)) {

        //this...
        this->_DisableUI();

        //UI...
        this->_instUI();
    
}

void ChatWidget::_instUI() {

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

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

void ChatWidget::_onRPZClientError(const QString &errMsg) {    
    
    //out log
    if(!this->serverName.isEmpty()) {
        const auto nm = errMsg + " (" + this->serverName + ")";
        this->_chatLog->writeAtEnd(nm, MessagesLog::MessageType::ServerLog);
    }

    this->_DisableUI();

}
void ChatWidget::_onReceivedMessage(const QVariantHash &message) {
    auto msg = RPZMessage(message);
    auto str_msg = msg.toString();
    
    //write in log
    this->_chatLog->writeAtEnd(str_msg);

}

void ChatWidget::_onReceivedLogHistory(const QVariantList &messages) {

    this->_EnableUI();

    //add list of messages
    for(auto &msg : messages) {
        this->_onReceivedMessage(msg.toHash());
    }

    //welcome msg
    const auto welcome = QString("Connecté au serveur (") + this->serverName + ")";
    this->_chatLog->writeAtEnd(welcome, MessagesLog::MessageType::ServerLog);
}

void ChatWidget::onRPZClientConnecting(RPZClient * cc) {

    ClientBindable::onRPZClientConnecting(cc);
    
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

    //on message typed 
    this->_chatEdit->disconnect();
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