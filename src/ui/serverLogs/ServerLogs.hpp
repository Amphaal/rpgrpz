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

#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>

#include "src/ui/_others/ConnectivityObserver.h"
#include "src/network/rpz/server/RPZServer.h"

class ServerLogs : public QDialog {
    Q_OBJECT

 public:
    explicit ServerLogs(RPZServer* toListenTo, QWidget * parent = nullptr) : QDialog(parent), _loglist(new QListWidget(this)) {
        this->setModal(true);
        this->setWindowTitle(tr("Server logs"));
        this->setVisible(false);
        this->setLayout(new QVBoxLayout);
        this->layout()->addWidget(this->_loglist);
    }

 private:
    QListWidget* _loglist = nullptr;
};
