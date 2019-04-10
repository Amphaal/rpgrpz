#include "LogContainer.h"

LogContainer::LogContainer(QWidget *parent) : QWidget(parent) {
    this->setLayout(new QVBoxLayout);
    this->layout()->setAlignment(Qt::AlignTop);
    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);
}

QWidget* LogContainer::writeAtEnd(const QString &newMessage) {
    
    //container
    auto line = new QWidget(this);
    line->setLayout(new QHBoxLayout);
    line->layout()->setContentsMargins(10, 3, 10, 3);
    line->setAutoFillBackground(true);

    //text part
    auto label = new QLabel(newMessage);
    label->setMargin(0);
    label->setWordWrap(true);
    line->layout()->addWidget(label);

    //add container to layout
    const auto widget_id = QUuid::createUuid();
    this->lines.insert(widget_id, line);
    this->idOfLine.insert(line, widget_id);

    //insert into view
    this->layout()->addWidget(line);

    return line;
}

void LogContainer::removeLine(const QUuid &idToRemove) {
    if(!this->lines.contains(idToRemove)) return;

    auto line = this->lines.take(idToRemove);
    this->idOfLine.take(line);
    line->deleteLater();
}