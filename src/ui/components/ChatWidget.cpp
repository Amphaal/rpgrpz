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

void ChatWidget::bindToChatClient(ChatClient * cc) {

    this->_currentCC = cc;
    this->_EnableUI();

    //initial message to log
    auto socketAddr = this->_currentCC->getConnectedSocketAddress();
    auto msg = QString("Connecté au serveur ") + socketAddr;
    this->printLog(msg.toStdString(), ChatWidget::LogType::ServerLog);

    //on error from client
    QObject::connect(this->_currentCC, &ChatClient::error, [&, socketAddr](const std::string errMsg) {
        
        //out log
        auto nm = errMsg + " (" + socketAddr.toStdString() + ")";
        this->printLog(nm, ChatWidget::LogType::ServerLog);

        this->_DisableUI();
    });

}

void ChatWidget::_DisableUI() {
    this->msgEdit->setPlaceholderText("");
    this->setEnabled(false);
}

void ChatWidget::_EnableUI() {
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
            colors->setColor(QPalette::Window, Qt::red);
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