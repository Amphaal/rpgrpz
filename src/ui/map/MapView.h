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
#include "src/network/rpz/client/RPZClient.h"

#include "src/ui/toysBox/ToysTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/shared/renderer/AtomConverter.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

#include "src/ui/map/modules/MV_Manipulation.hpp"
#include "src/ui/map/modules/MV_HUDLayout.hpp"

#include "src/shared/renderer/DrawingAssist.hpp"

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
        void _onUIUpdateRequest(const QHash<QGraphicsItem*, RPZAtom::Updates> &toUpdate);
        void _onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const RPZAtom::Updates &updates);
        void _onOwnershipChanged(const QList<QGraphicsItem*> changing, bool owned);

        //network
        void _sendMapHistory();
        void _onIdentityReceived(const RPZUser &self);

    private:
        RPZMapParameters _currentMapParameters;
        DrawingAssist* _drawingAssist = nullptr;
        AtomsContextualMenuHandler* _menuHandler = nullptr;
        AtomActionsHandler* _atomActionsHandler = nullptr;

        //helpers
        void _handleHintsSignalsAndSlots();
        void _updateItemValue(QGraphicsItem* item, const RPZAtom::Updates &updates);
        bool _preventMoveOrInsertAtPosition(QGraphicsItem *toCheck, const QPointF &toCheckAt = QPointF());
        void _addItemToScene(QGraphicsItem* item);

        //Selection
            bool _ignoreSelectionChangedEvents = false;
            bool _isAnySelectableItemsUnderCursor(const QPoint &cursorPosInWindow) const;
            void _notifySelection();

        //ownership
            void _configureOwnership(const QList<QGraphicsItem*> &toConfigure, bool owns);

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