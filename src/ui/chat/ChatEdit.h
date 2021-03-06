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
#include <QtWidgets/QPushButton>
#include <QString>
#include <QHBoxLayout>
#include <QEvent>
#include <QCompleter>
#include <QStringListModel>

#include "src/ui/chat/LineEditHistoriable.hpp"
#include "src/ui/_others/ConnectivityObserver.h"

class ChatEdit : public QWidget, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit ChatEdit(QWidget * parent = nullptr);

 signals:
    void askedToSendCommand(const QString &command, bool isDiceThrowCommand);

 protected:
    void changeEvent(QEvent *event) override;
    void connectingToServer() override;
    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override;

 private:
    LineEditHistoriable* _msgEdit = nullptr;
    QPushButton* _sendMsgBtn = nullptr;
    QPushButton* _useDicerBtn = nullptr;
    QCompleter* _completer = nullptr;

    void _sendCommand();
    void _defineMsgSendBtn();

    bool _explicitlyRequestingThrowCommand() const;

    void _onWhisperTargetsChanged();
};
