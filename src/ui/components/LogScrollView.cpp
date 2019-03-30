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

}

void LogScrollView::writeAtEnd(const std::string & newMessage, QPalette* colorPalette) {
    if(!this->_log) return;

    this->_log->writeAtEnd(newMessage, colorPalette);

}

void LogScrollView::newLog() {

    if(!this->_log) {
        delete this->_log;
        this->_log = 0;
    }

    this->_log = new LogView;
    this->setWidget(this->_log);

}


void LogScrollView::_scrollUpdate() {
    //to perform heavy CPU consuming action
    auto tabScrollBar = this->verticalScrollBar();
    tabScrollBar->setValue(tabScrollBar->maximum());
}
