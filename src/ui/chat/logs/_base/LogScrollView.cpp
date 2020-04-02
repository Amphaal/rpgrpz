// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

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
