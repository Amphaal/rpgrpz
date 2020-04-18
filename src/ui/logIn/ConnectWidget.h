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

#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>
#include <QComboBox>

#include "src/helpers/_appContext.h"

#include "src/shared/hints/HintThread.hpp"

#include "src/shared/database/CharactersDatabase.h"
#include "src/ui/_others/ConnectivityObserver.h"

class ConnectWidget : public QWidget, public ConnectivityObserver {

    Q_OBJECT

 public:
        enum class State { NotConnected, Connecting, Connected };
        Q_ENUM(State)

        ConnectWidget(QWidget *parent = nullptr);

    protected:
        void connectingToServer() override;
        void connectionClosed(bool hasInitialMapLoaded) override;

 private:
        QLineEdit* _nameTarget = nullptr;
        QLineEdit* _domainTarget = nullptr;
        QComboBox* _characterSheetTarget = nullptr;
        
        QPushButton* _connectBtn = nullptr;

        ConnectWidget::State _state = ConnectWidget::State::NotConnected;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _changeState(ConnectWidget::State newState);

        void _onConnectButtonPressed();
        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onGameSessionReceived(const RPZGameSession &gameSession);

        void _saveValuesAsSettings();

        void _fillCharacterSheetCombo();
        RPZCharacter::Id _getSelectedCharacterId();
};  