#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : 
            QGroupBox(parent),
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
    QSettings settings;
    auto splitter = new RestoringSplitter("ChatWidgetSplitter");
    splitter->addWidget(_left());
    splitter->addWidget(_right());
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->loadState();
    this->layout()->addWidget(splitter);
}

void ChatWidget::_onRPZClientError(const std::string errMsg) {    
    
    //out log
    if(!this->serverName.isEmpty()) {
        auto nm = errMsg + " (" + this->serverName.toStdString() + ")";
        this->writeInChatLog(nm, ChatWidget::LogType::ServerLog);
    }

    this->_DisableUI();

}
void ChatWidget::_onRPZClientReceivedMessage(const std::string message) {
    this->writeInChatLog(message);
}

void ChatWidget::_onRPZClientReceivedHistory() {
    auto msg = QString("Connecté au serveur (") + this->serverName + ")";
    this->writeInChatLog(msg.toStdString(), ChatWidget::LogType::ServerLog);
}

void ChatWidget::bindToRPZClient(RPZClient * cc) {

    this->_currentCC = cc;
    this->serverName = cc->getConnectedSocketAddress();
    this->_usersLog->newLog();
    this->_chatLog->newLog();

    //on error from client
    QObject::connect(
        this->_currentCC, &RPZClient::error, 
        this, &ChatWidget::_onRPZClientError
    );
    
    //on message received
    QObject::connect(
        this->_currentCC, &RPZClient::receivedMessage, 
        this, &ChatWidget::_onRPZClientReceivedMessage
    );

    //welcome once all history have been received
    QObject::connect(
        this->_currentCC, &RPZClient::historyReceived, 
        this, &ChatWidget::_onRPZClientReceivedHistory
    );

    //ss
    QObject::connect(
        this->_currentCC, &RPZClient::loggedUsersUpdated,
        this, &ChatWidget::_onRPZClientloggedUsersUpdated
    );

    //enable UI at connection
    QObject::connect(
        this->_currentCC, &RPZClient::historyReceived, 
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
    this->_chatEdit->setEnabled(false);
    this->_usersLog->setEnabled(false);
    this->_chatLog->setEnabled(false);
}

void ChatWidget::_EnableUI() {
    this->_chatEdit->setEnabled(true);
    this->_usersLog->setEnabled(true);
    this->_chatLog->setEnabled(true);
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

void ChatWidget::_onRPZClientloggedUsersUpdated(QVariantList users) {
    this->_usersLog->newLog();
    for(auto user : users) {
        auto un = user.toString().toStdString();
        this->_usersLog->writeAtEnd(un, nullptr, &QPixmap(":/icons/app/user.png"));
    }
}