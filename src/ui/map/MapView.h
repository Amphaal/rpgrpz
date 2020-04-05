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
#include "src/ui/_others/ConnectivityObserver.h"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/ui/toysBox/ToysTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/shared/renderer/AtomConverter.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

#include "src/ui/map/modules/MV_Manipulation.hpp"
#include "src/ui/map/modules/MV_HUDLayout.hpp"

#include "src/shared/renderer/assists/DrawingAssist.hpp"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

class MapView : public QGraphicsView, public ConnectivityObserver, public MV_Manipulation, public MV_HUDLayout, public AtomSelector {

    Q_OBJECT

    public:
        MapView(QWidget *parent = nullptr);
        
        const QList<RPZAtom::Id> selectedIds() const override;

        void scrollFromMinimap(QWheelEvent *event);
        void focusFromMinimap(const QPointF &scenePoint);

    public slots:
        void onActionRequested(const MapAction &action);
        void onHelperActionTriggered(QAction *action);
    
    signals:
        void remoteChanged(bool isRemote);
        void cameraMoved();
        void requestingFocusOnCharacter(const RPZCharacter::Id &characterIdToFocus);

    protected:
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;

        void connectingToServer() override;
        void connectionClosed(bool hasInitialMapLoaded) override;

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

    private slots:
        void _onUIAlterationRequest(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter);
        void _onUIAlterationRequest(const Payload::Alteration &type, const OrderedGraphicsItems &toAlter);
        void _onUIUpdateRequest(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate);
        void _onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates, bool isPreview);
        void _onOwnershipChanged(const QList<QGraphicsItem*> changing, bool owned);
        void _onFogModeChanged(const RPZFogParams::Mode &newMode);
        void _onFogChanged(const QList<QPolygonF> &updatedFog);

        //network
        void _sendMapHistory();
        void _onGameSessionReceived(const RPZGameSession &gameSession);

    private:
        RPZMapParameters _currentMapParameters;
        DrawingAssist* _drawingAssist = nullptr;
        AtomsContextualMenuHandler* _menuHandler = nullptr;
        AtomActionsHandler* _atomActionsHandler = nullptr;

        //helpers
        void _handleHintsSignalsAndSlots();
        void _metadataUpdatePostProcess(const QList<QGraphicsItem*> &FoWSensitiveItems);

        bool _preventMoveOrInsertAtPosition(QGraphicsItem *toCheck, const QPointF &toCheckAt = QPointF());
        void _addItemToScene(QGraphicsItem* item);

        //fog
            bool _mayFogUpdateAtoms(const MapViewFog::FogChangingVisibility &itemsWhoChanged) const;

        //Selection
            bool _ignoreSelectionChangedEvents = false;
            bool _isAnySelectableItemsUnderCursor(const QPoint &cursorPosInWindow) const;
            void _notifySelection();

        //ownership
            void _configureOwnership(const QList<QGraphicsItem*> &toConfigure, bool owns);
            void _mightUpdateTokens();

        //ghost
            QGraphicsItem* _displayableGhostItem();
            void _mightCenterGhostWithCursor();

        //registered points
            bool _isMousePressed = false;
            bool _isCursorIn = false;

        //tool
            MapTool _tool = MapTool::Default;
            MapTool _quickTool = MapTool::Default;
            MapTool _getCurrentTool() const;
            void _changeTool(MapTool newTool, bool quickChange = false);
            void _resetTool();
        
        //walking...
            QCursor _walkingCursor;
            MapViewWalkingHelper* _walkingHelper = nullptr;
            QGraphicsItem* _toWalk = nullptr;
            void _mightUpdateWalkingHelperPos();
            void _clearWalkingHelper();
            bool _tryToInvokeWalkableHelper(QGraphicsItem * toBeWalked);

        void onAnimationManipulationTickDone() override;
        void onViewRectChange() override;
        
};