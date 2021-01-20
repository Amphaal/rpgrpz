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

#include <QGraphicsView>

#include "src/shared/hints/HintThread.hpp"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

#include "src/helpers/VectorSimplifier.hpp"

class QuickDrawingAssist : public QObject, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit QuickDrawingAssist(QGraphicsView* view) : _view(view) {
        this->_tmQDUpdater.setInterval(1000);
        QObject::connect(
            &this->_tmQDUpdater, &QTimer::timeout,
            this, &QuickDrawingAssist::_sendQuickDrawNetworkUpdate
        );
    }

    void addDrawingPoint(const QPoint &cursorPosInWindow) {
        auto scenePos = this->_view->mapToScene(cursorPosInWindow);

        this->_resetTempDrawing();
        this->_beginDrawing(scenePos);

        // begin feeding qd to network if allowed
        if (this->_allowNetworkInteraction()) {
            this->_tmQDUpdater.start();
        }
    }

    void updateDrawingPath(const QPoint &evtPoint) {
        // if no temp, stop
        if (!this->_tempDrawing) return;

        // define destination coordonate
        auto sceneCoord = this->_view->mapToScene(evtPoint);
        auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

        this->_tempDrawing->moveLine(pathCoord);
    }

    void onMouseRelease() {
        // stop sending data to network
        this->_tmQDUpdater.stop();

        // may send last bits to network
        this->_maySendQuickDrawNetworkUpdate(true);

        this->_resetTempDrawing();
    }

    void clearDrawings() {
        for (auto i : this->_quickDrawings) {
            i->deleteLater();
        }
        this->_resetTempDrawing();
    }

 protected:
    void connectingToServer() override {
        QObject::connect(
            this->_rpzClient, &RPZClient::gameSessionReceived,
            this, &QuickDrawingAssist::_defineSelfUserFromSession
        );

        QObject::connect(
            this->_rpzClient, &RPZClient::quickDrawBitsReceived,
            this, &QuickDrawingAssist::_onQuickDrawBitsReceived
        );
    }
    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override {
        this->_currentUser.clear();

        // stop sending data to network
        this->_tmQDUpdater.stop();

        // everything but temporary must be registered to deletion
        for (auto qdi : _quickDrawings) {
            if (qdi == this->_tempDrawing) continue;
            qdi->registerForDeletion();
        }
    }

 private:
    QGraphicsView* _view = nullptr;
    QuickDrawItem* _tempDrawing = nullptr;
    QHash<RPZQuickDrawBits::Id, QuickDrawItem*> _quickDrawings;
    QTimer _tmQDUpdater;
    RPZUser _currentUser;

    void _defineSelfUserFromSession(const RPZGameSession &gameSession) {
        Q_UNUSED(gameSession);
        this->_currentUser = this->_rpzClient->identity();
    }

    bool _allowNetworkInteraction() const {
        return _tempDrawing && _rpzClient && !_currentUser.isEmpty();
    }

    void _sendQuickDrawNetworkUpdate() {
        return _maySendQuickDrawNetworkUpdate(false);
    }

    void _maySendQuickDrawNetworkUpdate(bool areLastFromCurrentQD) {
        // prevent
        if (!this->_allowNetworkInteraction()) return;

        // prepare
        auto bits = this->_tempDrawing->dequeuePushPoints();
        if(!bits.elementCount()) return;

        RPZQuickDrawBits qd(
            this->_tempDrawing->pos(),
            this->_tempDrawing->id(),
            this->_currentUser.id(),
            bits,
            areLastFromCurrentQD
        );

        // send to network
        QMetaObject::invokeMethod(this->_rpzClient, "sendQuickdraw",
            Q_ARG(RPZQuickDrawBits, qd)
        );
    }

    void _onQuickDrawBitsReceived(const RPZQuickDrawBits &qd) {
        auto bits = qd.bitsAsPath();

        // find in list
        auto drawId = qd.drawId();
        auto found = this->_quickDrawings.value(drawId);

        // if not create it
        if (!found) {
            auto associatedUser = this->_rpzClient->sessionUsers().value(qd.drawerId());
            auto startPos = qd.scenePos();
            found = _createQuickDraw(associatedUser, startPos, drawId);
        }

        // add bits
        found->addPathBits(bits, qd.areLastBits());
    }

    void _beginDrawing(const QPointF &scenePos) {
        this->_tempDrawing = _createQuickDraw(this->_currentUser, scenePos);
    }

    QuickDrawItem* _createQuickDraw(const RPZUser &emiter, const QPointF &startPos, const RPZQuickDrawBits::Id &drawId = 0) {
        // create item
        auto item = new QuickDrawItem(emiter, startPos, drawId);
        auto _t_Id = item->id();

        // remove from hash once destroyed
        QObject::connect(
            item, &QObject::destroyed,
            [=](){
                this->_quickDrawings.remove(_t_Id);
        });

        // add to list
        this->_quickDrawings.insert(
            item->id(),
            item
        );

        // add to scene
        this->_view->scene()->addItem(item);

        return item;
    }

    void _resetTempDrawing() {
        if (this->_tempDrawing) this->_tempDrawing->registerForDeletion();
        this->_tempDrawing = nullptr;
    }
};
