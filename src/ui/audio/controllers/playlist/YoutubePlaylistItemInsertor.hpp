// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QWidget>
#include <QLineEdit>
#include <QToolTip>
#include <QPushButton>

#include <QHBoxLayout>
#include <QIcon>
#include <QRegularExpression>

#include <audiotube/VideoMetadata.h>

#include "src/helpers/_appContext.h"

class YoutubePlaylistItemInsertor : public QWidget {
    Q_OBJECT

 signals:
    void insertionRequested(const QString &ytUrl);

 private:
    QLineEdit* _ytUrlEdit = nullptr;
    QPushButton* _insertLinkBtn = nullptr;

    void _handleLinkInsertionAttempt() {
        auto url = this->_ytUrlEdit->text();
        this->_ytUrlEdit->clear();

        auto matches = AudioTube::Regexes::YoutubeIdFinder.match(url.toStdString());

        if (!matches) {
            QToolTip::showText(this->_ytUrlEdit->mapToGlobal(QPoint()), tr("Invalid Youtube URL !"));
            return;
        }

        emit insertionRequested(url);
    }

 public:
    explicit YoutubePlaylistItemInsertor(QWidget *parent = nullptr) : QWidget(parent),
        _ytUrlEdit(new QLineEdit),
        _insertLinkBtn(new QPushButton) {
        this->setLayout(new QHBoxLayout);

        this->_ytUrlEdit->setPlaceholderText(tr("Paste here a Youtube URL..."));
        this->_ytUrlEdit->setToolTip(this->_ytUrlEdit->placeholderText());
        QObject::connect(
            this->_ytUrlEdit, &QLineEdit::returnPressed,
            this, &YoutubePlaylistItemInsertor::_handleLinkInsertionAttempt
        );

        this->_insertLinkBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/add.png")));
        this->_insertLinkBtn->setToolTip(tr("Click to add to playlist"));

        QObject::connect(
            this->_insertLinkBtn, &QPushButton::clicked,
            this, &YoutubePlaylistItemInsertor::_handleLinkInsertionAttempt
        );

        this->layout()->addWidget(this->_ytUrlEdit);
        this->layout()->addWidget(this->_insertLinkBtn);
    }
};
