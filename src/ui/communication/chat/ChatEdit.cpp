#include "ChatEdit.h"

ChatEdit::ChatEdit(QWidget * parent) : QWidget(parent), 
    _msgEdit(new QLineEdit), 
    _sendMsgBtn(new QPushButton("Envoyer Message")) {
    
    //layout
    this->setLayout(new QHBoxLayout);
    this->layout()->setMargin(0);
    this->layout()->addWidget(this->_msgEdit);
    this->layout()->addWidget(this->_sendMsgBtn);

    //on click
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

void ChatEdit::onRPZClientConnecting(RPZClient* cc) {
    
    ClientBindable::onRPZClientConnecting(cc);

    //on user connected
    QObject::connect(
        cc, &RPZClient::loggedUsersUpdated,
        this, &ChatEdit::_onUsersChanged
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

    auto textCommand = this->_msgEdit->text();

    //check if is sendable
    if(!MessageInterpreter::isSendable(textCommand)) return;

    //empty input and ask for send
    this->_msgEdit->setText("");
    emit askedToSendMessage(textCommand);
}

void ChatEdit::_onUsersChanged(const QVariantList &users) {
           
    RPZList<RPZUser> list(users);

    QSet<QString> usernamesList;
    for(auto &user : list) {
        auto adapted = MessageInterpreter::usernameToCommandCompatible(user.name());
        usernamesList.insert("@" + adapted);
    }

    auto model = new QStringListModel(usernamesList.toList());
    auto completer = new QCompleter(model);
    completer->setCompletionMode(QCompleter::CompletionMode::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchFlag::MatchContains);
    
    this->_msgEdit->setCompleter(completer);
}