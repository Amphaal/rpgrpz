#include "LogView.h"

LogView::LogView(QWidget *parent) : QWidget(parent) {
    this->setLayout(new QVBoxLayout);
    this->layout()->setAlignment(Qt::AlignTop);
    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);
}

QUuid LogView::writeAtEnd(const std::string & newMessage, QPalette* colorPalette, QPixmap* pixAsIcon) {
    
    //container
    auto line = new QWidget;
    line->setLayout(new QHBoxLayout);
    line->layout()->setContentsMargins(10, 3, 10, 3);
    line->setAutoFillBackground(true);

    //logo part
    if(pixAsIcon) {
        auto icon = new QLabel; 
        icon->setMargin(0); 
        icon->setPixmap(pixAsIcon->scaled(14, 14));
        icon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        line->layout()->addWidget(icon);
    }

    //text part
    auto msg = QString::fromStdString(newMessage);
    auto label = new QLabel(msg);
    label->setMargin(0);
    label->setWordWrap(true);
    line->layout()->addWidget(label);
    
    if(colorPalette) {
        line->setPalette(*colorPalette);
    }

    //add container to layout
    auto id = QUuid::createUuid();
    this->_lines.insert(id, line);
    this->layout()->addWidget(line);

    return id;
}

void LogView::removeLine(QUuid idToRemove) {
    if(!this->_lines.contains(idToRemove)) return;

    auto line = this->_lines[idToRemove];
    line->deleteLater();

    this->_lines.remove(idToRemove);
}