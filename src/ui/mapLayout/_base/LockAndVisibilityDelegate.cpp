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

#include "LockAndVisibilityDelegate.h"

LockAndVisibilityDelegate::LockAndVisibilityDelegate(QWidget *parent) : QStyledItemDelegate(parent) {
    if (!_hiddenPix && !_lockPix) {
        _hiddenPix = new QPixmap(QStringLiteral(u":/icons/app/tools/hidden.png"));
        _lockPix = new QPixmap(QStringLiteral(u":/icons/app/tools/lock.png"));
    }
}

void LockAndVisibilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    // fetch data
    auto item = MapLayoutItem::fromIndex(index);
    if (!item) return;

    // check if atom
    auto atom = dynamic_cast<MapLayoutAtom*>(item);
    if (!atom) return;

    // may draw "hide" icon
    if (atom->isHidden()) {
        painter->drawPixmap(option.rect.topLeft(), *_hiddenPix);
    }

    // may draw "lock" icon
    if (atom->isLocked()) {
        auto startPoint = QPoint(
            option.rect.right() - 16,
            option.rect.top()
        );
        painter->drawPixmap(startPoint, *_lockPix);
    }
}

QSize LockAndVisibilityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(4, 4);
}
