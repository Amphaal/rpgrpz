#include "LogScrollView.h"

LogScrollView::LogScrollView(QWidget *parent) : QScrollArea(parent) {
    
    this->setWidgetResizable(true);
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);    
    this->setAutoFillBackground(true);

    //bindings...
    QObject::connect(
        this->verticalScrollBar(), &QScrollBar::rangeChanged,
        this, &LogScrollView::_scrollUpdate
    );

}

void LogScrollView::_scrollUpdate() {
    //to perform heavy CPU consuming action
    auto tabScrollBar = this->verticalScrollBar();
    tabScrollBar->setValue(tabScrollBar->maximum());
}
