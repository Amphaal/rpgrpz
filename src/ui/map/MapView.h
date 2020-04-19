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

#include <QtMath>
#include <QMetaObject>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QWidget>
#include <QTimer>

#include <QWheelEvent>
#include <QTimeLine>
#include <QGLWidget>
#include <QCursor>
#include <QScrollBar>
#include <QVariant>
#include <QVector>

#include "src/shared/renderer/AtomRenderer.h"

#include "src/ui/map/toolbars/MapTools.hpp"
#include "src/ui/map/_base/AnimationTimeLine.hpp"

#include "src/shared/hints/HintThread.hpp"
#include "src/shared/models/RPZAtom.h"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/ui/toysBox/ToysTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/shared/renderer/AtomConverter.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

#include "src/ui/map/modules/MV_Manipulation.hpp"
#include "src/ui/map/modules/MV_HUDLayout.hpp"

#include "src/shared/renderer/assists/AtomDrawingAssist.hpp"
#include "src/shared/renderer/assists/QuickDrawingAssist.hpp"
#include "src/shared/renderer/assists/PingAssist.hpp"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

class MapView : public QGraphicsView, public MV_Manipulation, public MV_HUDLayout, public AtomSelector {
    Q_OBJECT

 public:
    explicit MapView(QWidget *parent = nullptr);

    const QList<RPZAtom::Id> selectedIds() const override;

    void scrollFromMinimap(QWheelEvent *event);
    void focusFromMinimap(const QPointF &scenePoint);
    void resetTool();

    void onActionRequested(const MapAction &action);
    void onHelperActionTriggered(QAction *action);

 signals:
    void cameraMoved();
    void requestingFocusOnCharacter(const RPZCharacter::Id &characterIdToFocus);

 protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

 private:
    RPZMapParameters _currentMapParameters;
    QuickDrawingAssist* _quickDrawingAssist = nullptr;
    AtomDrawingAssist* _atomDrawingAssist = nullptr;
    PingAssist* _pingAssist = nullptr;
    AtomsContextualMenuHandler* _menuHandler = nullptr;
    AtomActionsHandler* _atomActionsHandler = nullptr;
    MeasurementHelper* _measurementHelper = nullptr;

    void _onUIAlterationRequest(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter);
    void _onUIAlterationRequest(const Payload::Alteration &type, const OrderedGraphicsItems &toAlter);
    void _onUIUpdateRequest(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate);
    void _onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates, bool isPreview);
    void _onOwnershipChanged(const QList<QGraphicsItem*> changing, bool owned);
    void _onFogModeChanged(const RPZFogParams::Mode &newMode);
    void _onFogChanged(const QList<QPolygonF> &updatedFog);

    // helpers
    void _handleHintsSignalsAndSlots();
    void _metadataUpdatePostProcess(const QList<QGraphicsItem*> &FoWSensitiveItems);
    void _addItemToScene(QGraphicsItem* item);

    // fog
        bool _mayFogUpdateAtoms(const MapViewFog::FogChangingVisibility &itemsWhoChanged) const;

    // Selection
        bool _ignoreSelectionChangedEvents = false;
        bool _isAnySelectableItemsUnderCursor(const QPoint &cursorPosInWindow) const;
        void _notifySelection();

    // ownership
        void _configureOwnership(const QList<QGraphicsItem*> &toConfigure, bool owns);

    // ghost
        QGraphicsItem* _displayableGhostItem();
        void _mightCenterGhostWithCursor();

    // registered points
        bool _isMousePressed = false;
        bool _isCursorIn = false;

    // tool
        MapTool _tool = MapTool::Default;
        MapTool _quickTool = MapTool::Default;
        MapTool _getCurrentTool() const;
        void _changeTool(MapTool newTool, bool quickChange = false);
        void _onToolRequested(const MapTool &tool, bool enabled);

    // icons
    QCursor _walkingCursor;
    QCursor _pingCursor;
    QCursor _quickDrawCursor;
    QCursor _measureCursor;

    // walking...
        WalkingHelper* _walkingHelper = nullptr;
        void _mightUpdateWalkingHelperPos();
        void _clearWalkingHelper();
        void _clearMeasurementHelper();
        bool _tryToInvokeWalkableHelper(const QList<QGraphicsItem*> &toBeWalked);

    void onAnimationManipulationTickDone() override;
    void onViewRectChange() override;
};
