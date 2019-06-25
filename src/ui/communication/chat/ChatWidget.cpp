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
        auto scroller = new LogScrollView(left);
        scroller->setWidget(this->_chatLog);
        left->layout()->addWidget(scroller);

        //messaging...
        left->layout()->addWidget(this->_chatEdit);
        
        return left;
    };
    
    ////////////////
    // right part //
    ////////////////

    auto _right = [&]() {
        this->_usersLog->setMinimumWidth(100);

        auto scroller = new LogScrollView(this);
        scroller->setWidget(this->_usersLog);
        return scroller;
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

void ChatWidget::_onRPZClientStatus(const QString &statusMsg, bool isError) {    
    
    auto respCode = isError ? RPZResponse::ResponseCode::Error : RPZResponse::ResponseCode::Status;

    //out log
    RPZResponse response(0, respCode, statusMsg);
    this->_chatLog->handleResponse(response);

    this->_DisableUI();

}

void ChatWidget::_onReceivedLogHistory(const QVariantList &messages) {

    this->_EnableUI();

    //add list of messages
    for(auto &msg : messages) {
        RPZMessage message(msg.toHash());
        this->_chatLog->handleMessage(message);
    }

    //welcome msg
    auto response = RPZResponse(NULL, RPZResponse::ResponseCode::ConnectedToServer, this->serverName);
    this->_chatLog->handleResponse(response);
}

void ChatWidget::onRPZClientConnecting(RPZClient * cc) {

    ClientBindable::onRPZClientConnecting(cc);
    
    this->serverName = cc->getConnectedSocketAddress();
    
    this->_usersLog->clearLines();
    this->_chatLog->clearLines();

    //on error from client
    QObject::connect(
        this->_rpzClient, &RPZClient::connectionStatus, 
        this, &ChatWidget::_onRPZClientStatus
    );
    
    //on message received
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedMessage, 
        [&](const QVariantHash &message) {
            auto msg = RPZMessage(message);
            this->_chatLog->handleMessage(msg);
        }
    );

    //welcome once all history have been received
    QObject::connect(
        this->_rpzClient, &RPZClient::receivedLogHistory, 
        this, &ChatWidget::_onReceivedLogHistory
    );

    //on server response
    QObject::connect(
        this->_rpzClient, &RPZClient::serverResponseReceived, 
        [&](const QVariantHash &reponse) {
            auto resp = RPZResponse(reponse);
            this->_chatLog->handleResponse(resp);
        }
    );

    
    //update users list
    QObject::connect(
        this->_rpzClient, &RPZClient::loggedUsersUpdated,
        this->_usersLog, &UsersLog::updateUsers
    );

    //on message send request 
    this->_chatEdit->disconnect();
    QObject::connect(
        this->_chatEdit, &ChatEdit::askedToSendMessage,
        [=](const QString &msg) {      
            RPZMessage message(msg);
            this->_chatLog->handleMessage(message, true);
            this->_rpzClient->sendMessage(message);
        }
        
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