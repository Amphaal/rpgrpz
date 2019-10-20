#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : 
            QWidget(parent),
            _chatLog(new MessagesLog),
            _chatEdit(new ChatEdit) {

        this->setEnabled(false);

        //UI...
        this->_instUI();
    
}

void ChatWidget::_instUI() {

    auto layout = new QVBoxLayout;
    layout->setMargin(0);
    
    auto scroller = new LogScrollView;
    scroller->setWidget(this->_chatLog);
    
    layout->addWidget(scroller);
    layout->addWidget(this->_chatEdit);

    this->setLayout(layout);
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);

}

void ChatWidget::_onRPZClientStatus(const QString &statusMsg, bool isError) {    
    
    auto respCode = isError ? RPZResponse::ResponseCode::Error : RPZResponse::ResponseCode::Status;

    //out log
    RPZResponse response(0, respCode, statusMsg);
    this->_chatLog->handleResponse(response);

    this->setEnabled(false);

}

void ChatWidget::_onReceivedLogHistory(const QVector<RPZMessage> &messages) {

    this->setEnabled(true);

    //add list of messages
    for(auto &msg : messages) {
        this->_chatLog->handleNonLocalMessage(msg);
    }

    //welcome msg
    auto response = RPZResponse(0, RPZResponse::ResponseCode::ConnectedToServer, this->serverName);
    this->_chatLog->handleResponse(response);
}

void ChatWidget::connectingToServer() {
    
    this->serverName = _rpzClient->getConnectedSocketAddress();
    
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