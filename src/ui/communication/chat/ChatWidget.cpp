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
    splitter->restore();
    this->layout()->addWidget(splitter);
    this->layout()->setMargin(0);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

void ChatWidget::_onRPZClientStatus(const QString &statusMsg, bool isError) {    
    
    auto respCode = isError ? RPZResponse::ResponseCode::Error : RPZResponse::ResponseCode::Status;

    //out log
    RPZResponse response(0, respCode, statusMsg);
    this->_chatLog->handleResponse(response);

    this->_DisableUI();

}

void ChatWidget::_onReceivedLogHistory(const QVector<RPZMessage> &messages) {

    this->_EnableUI();

    //add list of messages
    for(auto &msg : messages) {
        this->_chatLog->handleNonLocalMessage(msg);
    }

    //welcome msg
    auto response = RPZResponse(0, RPZResponse::ResponseCode::ConnectedToServer, this->serverName);
    this->_chatLog->handleResponse(response);
}

void ChatWidget::onRPZClientConnecting() {
    
    this->serverName = _rpzClient->getConnectedSocketAddress();
    
    this->_usersLog->clearLines();
    this->_chatLog->clearLines();

    //on error from client
    QObject::connect(
        _rpzClient, &RPZClient::connectionStatus, 
        this, &ChatWidget::_onRPZClientStatus
    );
    
    //on message received
    QObject::connect(
        _rpzClient, &RPZClient::receivedMessage, 
        this->_chatLog, &MessagesLog::handleNonLocalMessage
    );

    //welcome once all history have been received
    QObject::connect(
        _rpzClient, &RPZClient::receivedLogHistory, 
        this, &ChatWidget::_onReceivedLogHistory
    );

    //on server response
    QObject::connect(
        _rpzClient, &RPZClient::serverResponseReceived, 
        this->_chatLog, &MessagesLog::handleResponse
    );

    
    //update users list
    QObject::connect(
        _rpzClient, &RPZClient::loggedUsersUpdated,
        this->_usersLog, &UsersLog::updateUsers
    );

    //on message send request 
    this->_chatEdit->disconnect();
    QObject::connect(
        this->_chatEdit, &ChatEdit::askedToSendMessage,
        [=](const QString &msg) {      
            RPZMessage message(msg);
            this->_chatLog->handleLocalMessage(message);
            QMetaObject::invokeMethod(this->_rpzClient, "sendMessage", Q_ARG(RPZMessage, message));
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