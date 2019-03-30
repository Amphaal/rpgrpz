#include "ChatWidget.h"

ChatWidget::ChatWidget(QWidget *parent) : 
            QWidget(parent),
            scrollArea(new QScrollArea),
            msgEdit(new QLineEdit),
            sendBtn(new QPushButton) {
        
        //UI...
        this->_instUI();

        
        //bindings...
        QObject::connect(
            this->scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged,
            this, &ChatWidget::_scrollUpdate
        );
    
}

void ChatWidget::_onChatClientError(const std::string errMsg) {    
    
    //out log
    if(!this->serverName.isEmpty()) {
        auto nm = errMsg + " (" + this->serverName.toStdString() + ")";
        this->printLog(nm, ChatWidget::LogType::ServerLog);
    }

    this->_DisableUI();

}
void ChatWidget::_onChatClientReceivedMessage(const std::string message) {
    this->printLog(message);
}

void ChatWidget::_onChatClientReceivedHistory() {
    auto msg = QString("Connecté au serveur (") + this->serverName + ")";
    this->printLog(msg.toStdString(), ChatWidget::LogType::ServerLog);
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
    this->msgEdit->setPlaceholderText("");
    this->setEnabled(false);
}

void ChatWidget::_EnableUI(QString serverAddress) {
    this->serverName = serverAddress;
    this->msgEdit->setPlaceholderText("Message à envoyer");
    this->msgEdit->setText("");
    this->setEnabled(true);
    this->createNewLog();
}

void ChatWidget::createNewLog() {
    if(!this->lsv) {
        delete this->lsv;
        this->lsv = 0;
    }
    this->lsv = new LogScrollView;
    this->scrollArea->setWidget(this->lsv);

};

void ChatWidget::printLog(const std::string &message, ChatWidget::LogType logType) {
    
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

    this->lsv->addMessage(message, colors);
};

void ChatWidget::_scrollUpdate() {
    //to perform heavy CPU consuming action
    auto tabScrollBar = this->scrollArea->verticalScrollBar();
    tabScrollBar->setValue(tabScrollBar->maximum());
}

void ChatWidget::_instUI() {
       
        //this...
        this->setLayout(new QVBoxLayout);
        this->layout()->setContentsMargins(10, 0, 10, 0);
        this->_DisableUI();
            
        //scroll
        this->scrollArea->setWidgetResizable(true);
        this->scrollArea->setAutoFillBackground(true);
        this->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        auto pal = this->scrollArea->palette();
        pal.setColor(QPalette::Background, Qt::white);
        this->scrollArea->setPalette(pal);
        this->layout()->addWidget(this->scrollArea);

        //messaging
        auto msgWdgt = new QWidget;
        msgWdgt->setLayout(new QHBoxLayout);
        msgWdgt->layout()->setMargin(0);
        this->sendBtn->setText("Envoyer Message");
        QObject::connect(
            this->msgEdit, &QLineEdit::returnPressed, 
            [&]() {
                this->sendBtn->click();
            }
        );
        QObject::connect(
            this->sendBtn, &QPushButton::clicked,
            this, &ChatWidget::_sendMessage
        );
        msgWdgt->layout()->addWidget(this->msgEdit);
        msgWdgt->layout()->addWidget(this->sendBtn);
        this->layout()->addWidget(msgWdgt);
}

void ChatWidget::_sendMessage() {
    auto msg = this->msgEdit->text();
    if(msg.isEmpty()) return;
    this->msgEdit->setText("");
    this->_currentCC->sendMessage(msg);
}