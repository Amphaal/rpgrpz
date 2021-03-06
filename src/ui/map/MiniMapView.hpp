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

#include <QPen>

#include <algorithm>

#include "src/ui/map/MapView.h"

class MiniMapView : public QGraphicsView {
 public:
    explicit MiniMapView(MapView* master, QWidget *parent = nullptr) : QGraphicsView(parent), _master(master) {
        this->setCursor(Qt::OpenHandCursor);

        // init
        this->_sceneToMimic = master->scene();
        this->_dummyScene = new QGraphicsScene;

        // visibility
        this->setVisible(false);
        this->_visibleAsap = AppContext::settings()->minimapActive();

        // events
        this->_handleHintsSignalsAndSlots();

        // viewport
        auto shared = (QGLWidget*)master->viewport();
        auto vp = new QGLWidget(shared->format(), nullptr, shared);
        this->setViewport(vp);

        // render hints
        this->setRenderHints(QPainter::Antialiasing);

        // hide scrollbars
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        this->setDragMode(QGraphicsView::NoDrag);
        this->setFixedSize(QSize(240, 240));
    }

    void setAsapVisibility(bool visible) {
        this->_visibleAsap = visible;
        if (!this->_alterationOngoing) {
            this->_generateMinimap();
        }
    }

 private slots:
    void _onHeavyAlterationStarted() {
        this->_alterationOngoing = true;
        this->setVisible(false);
    }

 private:
    QGraphicsScene* _dummyScene = nullptr;
    QGraphicsScene* _sceneToMimic = nullptr;
    MapView* _master = nullptr;
    bool _visibleAsap = false;
    bool _alterationOngoing = false;
    QPixmap _cachedMinimap;

    bool _isMousePressed = false;

    static constexpr QRectF DEFAULT_MINIMAP_RECT { QPointF(-320, -320), QSizeF(640, 640) };

    QRectF _getMinimumSceneRect() {
        // get bounds of map, unit it with default
        auto minimapRect = this->_sceneToMimic->itemsBoundingRect();
        minimapRect = minimapRect.united(DEFAULT_MINIMAP_RECT);  // unite

        // unit with reversed
        auto reversed = QRectF(-minimapRect.bottomRight(), -minimapRect.topLeft());
        minimapRect = minimapRect.united(reversed);

        // check rect size
        auto rectSize = minimapRect.size();
        auto rectWidth = rectSize.width();
        auto rectHeight = rectSize.height();

        // turn into square
        if (rectHeight != rectWidth) {
            auto max = std::max(rectWidth, rectHeight);
            minimapRect.setSize({max, max});
        }

        // move to center of scene, eg (0, 0)
        minimapRect.moveCenter({});

        return minimapRect;
    }

    void _generateMinimap() {
        // set visibility
        this->setVisible(this->_visibleAsap);

        // fit current in view
        this->setScene(this->_sceneToMimic);
        auto minimumRect = this->_getMinimumSceneRect();
        this->fitInView(minimumRect, Qt::AspectRatioMode::KeepAspectRatio);

        // snapshot
        this->_cachedMinimap = this->grab();

        // display dummy scene
        this->_dummyScene->setSceneRect(this->_sceneToMimic->sceneRect());
        this->setScene(this->_dummyScene);
        this->fitInView(minimumRect, Qt::AspectRatioMode::KeepAspectRatio);
    }

    void _handleHintsSignalsAndSlots() {
        // on map loading, set/unset placeholder...
        QObject::connect(
            HintThread::hint(), &MapHint::heavyAlterationStarted,
            this, &MiniMapView::_onHeavyAlterationStarted
        );
        QObject::connect(
            this->_master, &MapView::heavyAlterationFinished,
            [=]() {
                this->_generateMinimap();
                this->_alterationOngoing = false;
        });

        //
        QObject::connect(
            this->_master, &MapView::cameraMoved,
            [=]() {
                this->setForegroundBrush(Qt::NoBrush);   // force foreground re-drawing
        });
    }

    void _drawViewIndic(QPainter *painter) {
        painter->save();

            // prevent transforms
            painter->setTransform(QTransform());

            // minimap
            painter->drawPixmap(QPointF(), this->_cachedMinimap);

            QPen pen;
            pen.setWidth(0);
            pen.setCosmetic(true);

            auto viewportMapRect = this->_master->mapToScene(
                this->_master->viewport()->rect()
            ).boundingRect();
            viewportMapRect = this->mapFromScene(viewportMapRect).boundingRect();

            // outer rect
            pen.setColor(Qt::black);
            painter->setPen(pen);
            painter->drawRect(viewportMapRect);

            // inner rect
            viewportMapRect = viewportMapRect.marginsRemoved(QMargins(1, 1, 1, 1));
            pen.setColor(Qt::white);
            painter->setPen(pen);
            painter->drawRect(viewportMapRect);

        painter->restore();
    }

    void _propagateFocusToMaster(QMouseEvent *event) {
        auto scenePos = this->mapToScene(event->pos());
        this->_master->focusFromMinimap(scenePos);
    }

 protected:
    void keyPressEvent(QKeyEvent *event) override {};

    void wheelEvent(QWheelEvent * event) override {
        this->_master->scrollFromMinimap(event);
    };

    void mousePressEvent(QMouseEvent *event) override {
        this->_isMousePressed = true;
        this->setCursor(Qt::ClosedHandCursor);
        this->_propagateFocusToMaster(event);
    };

    void mouseReleaseEvent(QMouseEvent *event) override {
        this->_isMousePressed = false;
        this->setCursor(Qt::OpenHandCursor);
    };

    void mouseMoveEvent(QMouseEvent *event) override {
        if (!this->_isMousePressed) return;
        this->_propagateFocusToMaster(event);
    }

    void drawForeground(QPainter *painter, const QRectF &rect) override {
        if (this->scene() == this->_sceneToMimic) return;
        this->_drawViewIndic(painter);
    }
};
