#include "ChatEdit.h"

ChatEdit::ChatEdit(QWidget * parent) : QWidget(parent), _msgEdit(new QLineEdit), _sendMsgBtn(new QPushButton) {
    
    this->setLayout(new QHBoxLayout);
    this->layout()->setMargin(0);
    this->_sendMsgBtn->setText("Envoyer Message");

    this->layout()->addWidget(this->_msgEdit);
    this->layout()->addWidget(this->_sendMsgBtn);

    QObject::connect(
        this->_sendMsgBtn, &QPushButton::clicked,
        this, &ChatEdit::_sendMessage
    );
    
    QObject::connect(
        this->_msgEdit, &QLineEdit::returnPressed, 
        [&]() {
            this->_sendMsgBtn->click();
        }
    );

}

void ChatEdit::changeEvent(QEvent *event) {
    if(event->type() != QEvent::EnabledChange) return;

    if(this->isEnabled()) {
        this->_msgEdit->setPlaceholderText("Message à envoyer");
        this->_msgEdit->setText("");
    } else {
        this->_msgEdit->setPlaceholderText("");
    }
}

void ChatEdit::_sendMessage() {
    auto msg = this->_msgEdit->text();
    if(msg.isEmpty()) return;
    this->_msgEdit->setText("");

    emit askedToSendMessage(msg);
}
