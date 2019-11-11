#include "ChatEdit.h"

ChatEdit::ChatEdit(QWidget * parent) : QWidget(parent), 
    _msgEdit(new QLineEdit), 
    _sendMsgBtn(new QPushButton(tr("Send Message"))) {
    
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

void ChatEdit::connectingToServer() {

    //on user connected
    QObject::connect(
        _rpzClient, &RPZClient::whisperTargetsChanged,
        this, &ChatEdit::_onWhisperTargetsChanged
    );

}

void ChatEdit::changeEvent(QEvent *event) {
    if(event->type() != QEvent::EnabledChange) return;

    if(this->isEnabled()) {
        this->_msgEdit->setPlaceholderText(tr(" Message to send"));
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

void ChatEdit::_onWhisperTargetsChanged() {
    
    QSet<QString> usernamesList;
    for(const auto &user : this->_rpzClient->sessionUsers()) {
        usernamesList.insert(QStringLiteral(u"@") + user.whisperTargetName());
    }

    auto model = new QStringListModel(usernamesList.toList());
    auto completer = new QCompleter(model);
    completer->setCompletionMode(QCompleter::CompletionMode::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchFlag::MatchContains);
    
    this->_msgEdit->setCompleter(completer);
}