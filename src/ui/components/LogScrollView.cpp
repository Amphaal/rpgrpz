#include "LogScrollView.h"

LogScrollView::LogScrollView(QWidget *parent) : QWidget(parent) {
    
    this->setLayout(new QVBoxLayout);
    this->layout()->setAlignment(Qt::AlignTop);
    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);
}

void LogScrollView::addMessage(const std::string & newMessage, QPalette* colorPalette) {
    
    auto msg = QString::fromStdString(newMessage);
    auto label = new QLabel(msg);
    label->setWordWrap(true);
    label->setAutoFillBackground(true);
    label->setContentsMargins(10, 3, 10, 3);
    
    if(colorPalette) {
        label->setPalette(*colorPalette);
    }

    this->limitLogSize();

    this->layout()->addWidget(label);
}

void LogScrollView::updateLatestMessage(const std::string & newMessage) {
    
    auto msg = QString::fromStdString(newMessage);
    auto i = this->layout()->count() - 1; //count items in layout
    
    //if no message, add message
    if(i < 0) {
        return this->addMessage(newMessage);
    }
    auto lbl = (QLabel*)this->layout()->itemAt(i)->widget();
    lbl->setText(msg);
}

void LogScrollView::limitLogSize() {
    if(this->layout()->count() > this->_maxLogMessages) {
        auto qli = this->layout()->takeAt(0);
        auto ltr = (QLabel*)qli->widget();
        ltr->setParent(nullptr);
        delete ltr;
        delete qli;
    }
}
