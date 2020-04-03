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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#include "LogContainer.h"

LogText::LogText(const QString &text) {
    
    this->setMargin(0);
    this->setWordWrap(true);
    this->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    this->setOpenExternalLinks(true);
    this->setTextFormat(Qt::RichText);

    //redefine text with rich hyperlinks
    auto withHyperlinks = text;
    QRegularExpression r(AppContext::REGEX_URL);
    auto matches = r.globalMatch(text);
    
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        auto url = match.captured();
        withHyperlinks.replace(url, QStringLiteral(u"<a href=\"%1\">%1</a>").arg(url));
    }

    this->setText(withHyperlinks);

}

LogItem::LogItem() : QWidget(), _hLayout(new QHBoxLayout) {
    this->setLayout(this->_hLayout);
    this->_hLayout->setContentsMargins(10, 3, 10, 3);
    this->_hLayout->setAlignment(Qt::AlignTop);
    this->setAutoFillBackground(true);
    this->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Maximum);
}

int LogItem::positionInLog() {
    return this->_position;
}

void LogItem::setPositionInLog(int position) {
    this->_position = position;
}

QHBoxLayout* LogItem::horizontalLayout() {
    return this->_hLayout;
}

LogContainer::LogContainer(QWidget *parent) : QWidget(parent), _vLayout(new QVBoxLayout) {
    this->setLayout(this->_vLayout);
    this->_vLayout->setAlignment(Qt::AlignTop);
    this->_vLayout->setSpacing(0);
    this->_vLayout->setMargin(0);
}

QVBoxLayout* LogContainer::_getLayout() {
    return this->_vLayout;
}

LogItem* LogContainer::_addLine(const Stampable &element, Stampable::Id putUnder) {
    
    auto eId = element.id();
    auto found = this->_linesBySerializableId.value(eId);

    //if non existant
    if(!found) {

        //create line
        found = new LogItem();

        //if no information about descendant, push last
        if(!putUnder) {
            this->layout()->addWidget(found);
        }
        else {
            //add carriage return symbol
            auto cr = new QLabel(QChar(0x2607));
            found->layout()->setAlignment(Qt::AlignLeft);
            found->layout()->addWidget(cr);

            //find position to put the line under
            auto targetPos = this->_linesBySerializableId.value(putUnder)->positionInLog();
            
            //add at pos
            this->_vLayout->insertWidget(targetPos + 1, found);
        }
        
        //set position
        found->setPositionInLog(this->layout()->count()-1);
        
        //add to store
        this->_linesBySerializableId.insert(eId, found);
    }

    return found;
}

LogItem* LogContainer::_getLine(const Stampable &element) {
    return this->_getLine(element.id());
}

LogItem* LogContainer::_getLine(Stampable::Id elementId) {
    return this->_linesBySerializableId.value(elementId);
}

void LogContainer::clearLines() {
    
    //clear UI
    while(auto item = this->layout()->takeAt(0)) {
        delete item->widget();
        this->layout()->removeItem(item);
    }

    //clear storage
    this->_linesBySerializableId.clear();
}