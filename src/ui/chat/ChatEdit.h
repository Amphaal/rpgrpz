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

#include <QWidget>
#include <QtWidgets/QPushButton>
#include <QLineEdit>
#include <QString>
#include <QHBoxLayout>
#include <QEvent>
#include <QCompleter>
#include <QStringListModel>

#include "src/ui/_others/ConnectivityObserver.h"

class ChatEdit : public QWidget, public ConnectivityObserver {

    Q_OBJECT

    public:
        ChatEdit(QWidget * parent = nullptr);
    
    signals:
        void askedToSendMessage(const QString &msg);
    
    protected:
        void changeEvent(QEvent *event) override;
        void connectingToServer() override;

    private:
        QLineEdit* _msgEdit = nullptr;
        QPushButton* _sendMsgBtn = nullptr;

        void _sendMessage();
        
        void _onWhisperTargetsChanged();
};