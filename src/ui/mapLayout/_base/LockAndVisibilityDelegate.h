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

#pragma once

#include <QPainter>

#include <QStyledItemDelegate>

#include "src/ui/mapLayout/model/MapLayoutAtom.h"

class LockAndVisibilityDelegate  : public QStyledItemDelegate {
    
    public:
        LockAndVisibilityDelegate(QWidget *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    private:
        static inline QPixmap* _hiddenPix = nullptr;
        static inline QPixmap* _lockPix = nullptr;
};