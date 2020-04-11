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

#pragma once

#include <QLabel>
#include <QHBoxLayout>
#include <QHash>
#include <QString>
#include <QWidget>
#include <QRegularExpression>

#include "src/shared/models/_base/Stampable.hpp"

class LogItem : public QWidget {
    
    public:
        LogItem(const Stampable::Id &boundMsgId);
        int positionInLog(); 
        void setPositionInLog(int position);
        QHBoxLayout* horizontalLayout();

        Stampable::Id boundMsgId() const;

    private:
        int _position = -1;
        Stampable::Id _boundMsgId;
        QHBoxLayout* _hLayout = nullptr;

};

class LogContainer : public QWidget {
    public:
        LogContainer(QWidget *parent = nullptr);
        void clearLines();
    
    protected:
        LogItem* _getLine(const Stampable &element);
        LogItem* _getLine(Stampable::Id elementId);
        LogItem* _addLine(const Stampable &element, Stampable::Id putUnder = 0);
        QVBoxLayout* _getLayout();
    
    private:
        QHash<Stampable::Id, LogItem*> _linesBySerializableId;
        QVBoxLayout* _vLayout = nullptr;
};