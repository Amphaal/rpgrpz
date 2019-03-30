#include "LogView.h"

LogView::LogView(QWidget *parent) : QWidget(parent) {
    
    this->setLayout(new QVBoxLayout);
    this->layout()->setAlignment(Qt::AlignTop);
    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);
}

QUuid LogView::writeAtEnd(const std::string & newMessage, QPalette* colorPalette) {
    
    auto msg = QString::fromStdString(newMessage);
    auto label = new QLabel(msg);
    label->setWordWrap(true);
    label->setAutoFillBackground(true);
    label->setContentsMargins(10, 3, 10, 3);
    
    if(colorPalette) {
        label->setPalette(*colorPalette);
    }

    this->layout()->addWidget(label);

    auto id = QUuid::createUuid();
    this->_labels.insert(id, label);

    return id;
}

void LogView::removeLine(QUuid idToRemove) {
    if(!this->_labels.contains(idToRemove)) return;

    auto lbl = this->_labels[idToRemove];
    lbl->deleteLater();

    this->_labels.remove(idToRemove);
}