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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QGraphicsView>

#include "src/shared/hints/HintThread.hpp"

#include "src/shared/renderer/graphics/MapViewGraphics.h"
#include "src/ui/audio/NotificationsAudioManager.hpp"

#include "src/shared/models/RPZPing.hpp"
#include "src/shared/renderer/assists/PingIndicatorItem.hpp"

class PingAssist : public QObject, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit PingAssist(QGraphicsView* view) : _view(view) {}

    void generatePing(const QPoint viewPosPoint) {
        auto scenePos = this->_view->mapToScene(viewPosPoint);

        if (this->_networkAllowed) {
            QMetaObject::invokeMethod(this->_rpzClient, "notifyPing",
                Q_ARG(QPointF, scenePos)
            );
        }

        auto user = this->_networkAllowed ? this->_rpzClient->identity() : RPZUser();
        this->_addPing(scenePos, user);
    }

 protected:
    virtual void connectingToServer() {
        QObject::connect(
            this->_rpzClient, &RPZClient::pingHappened,
            this, &PingAssist::_addPingFromNetwork
        );

        QObject::connect(
            this->_rpzClient, &RPZClient::gameSessionReceived,
            this, &PingAssist::_onGameSessionReceived
        );
    }
    virtual void connectionClosed(bool hasInitialMapLoaded) {
        this->_networkAllowed = false;
    }

 private:
    bool _networkAllowed = false;
    QGraphicsView* _view = nullptr;

    void _onGameSessionReceived(const RPZGameSession &gameSession) {
        Q_UNUSED(gameSession);
        this->_networkAllowed = true;
    }

    void _addPingFromNetwork(const RPZPing &ping) {
        auto user = this->_rpzClient->sessionUsers().value(ping.emiterId());
        this->_addPing(ping.scenePos(), user);
    }

    void _addPing(const QPointF &scenePosPoint, RPZUser &user) {
        NotificationsAudioManager::get()->playPing();

        auto ping = new PingItem(scenePosPoint, user.color());
        this->_view->scene()->addItem(ping);

        auto indicator = new PingIndicatorItem(ping, this->_view);
        this->_view->scene()->addItem(indicator);
    }
};
