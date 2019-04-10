#include "LogScrollView.h"

LogScrollView::LogScrollView(QWidget *parent) : QScrollArea(parent) {
    
    this->setWidgetResizable(true);
    this->setAutoFillBackground(true);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    auto pal = this->palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setPalette(pal);

    //bindings...
    QObject::connect(
        this->verticalScrollBar(), &QScrollBar::rangeChanged,
        this, &LogScrollView::_scrollUpdate
    );

    this->newLog();

}

QWidget* LogScrollView::writeAtEnd(const QString &newMessage) {
    
    if(!this->_log) return nullptr;

    return this->_log->writeAtEnd(newMessage);
}

void LogScrollView::newLog() {
    this->clearLog();
    this->_log = new LogContainer;
    this->setWidget(this->_log);

}

void LogScrollView::clearLog() {
    if(this->_log) {
        delete this->_log;
        this->_log = 0;
    }
}


void LogScrollView::_scrollUpdate() {
    //to perform heavy CPU consuming action
    auto tabScrollBar = this->verticalScrollBar();
    tabScrollBar->setValue(tabScrollBar->maximum());
}
