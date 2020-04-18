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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QLabel>
#include "src/helpers/_appContext.h"

class LogText : public QLabel {

    public:
        LogText(const QString &txt, QWidget* parent = nullptr) : QLabel(parent) {
        
            this->setMargin(0);
            this->setWordWrap(true);
            this->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
            this->setOpenExternalLinks(true);
            this->setTextFormat(Qt::RichText);

            //redefine text with rich hyperlinks
            auto withHyperlinks = txt;
            QRegularExpression r(AppContext::REGEX_URL);
            auto matches = r.globalMatch(withHyperlinks);
            
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                auto url = match.captured();
                withHyperlinks.replace(url, QStringLiteral(u"<a href=\"%1\">%1</a>").arg(url));
            }

            this->setText(withHyperlinks);
            
        }

};
