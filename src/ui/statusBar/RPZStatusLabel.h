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

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#include <QString>
#include <QMovie>



class RPZStatusLabel : public QWidget {

    Q_OBJECT

    public:
        enum State {Finished = 0, Waiting, Processing, Error };

        RPZStatusLabel(const QString &description, QWidget* parent = nullptr);

        void updateState(const QString &stateText, RPZStatusLabel::State state = Finished);
        void setWaiting(bool waiting);
        QLabel* dataLabel();

    private:
        QLabel* _descriptionLbl = nullptr;
        QLabel* _dataLbl = nullptr;
        QLabel* _loaderLbl = nullptr;
        QMovie* _loader = nullptr;
       
};