#pragma once

#include <QtMath>
#include <QMetaObject>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QWidget>
#include <QTimer>

#include <QWheelEvent>
#include <QTreeWidgetItem>
#include <QTimeLine>
#include <QGLWidget>
#include <QCursor>
#include <QScrollBar>
#include <QVariant>
#include <QVector>

#include "graphics/CustomGraphicsItemHelper.h"

#include "MapTools.h"
#include "base/AnimationTimeLine.hpp"

#include "src/shared/atoms/MapHint.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/others/ClientBindable.h"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"

#include "src/ui/assets/base/AssetsTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/shared/models/RPZToyMetadata.h"
#include "src/helpers/AtomConverter.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class MapView : public QGraphicsView, public ClientBindable, public ItemChangedNotified {

    Q_OBJECT

    public:
        enum Tool { Default, Atom, Scroll };

        MapView(QWidget *parent);
        ~MapView();

        MapHint* hints() const;

    public slots:
        void onActionRequested(const MapTools::Actions &action);
    
    signals:
        void remoteChanged(bool isRemote);

    protected:
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;

        void onRPZClientConnecting() override;
        void onRPZClientDisconnect() override;

        void contextMenuEvent(QContextMenuEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;
        void resizeEvent(QResizeEvent * event) override;
        void mouseDoubleClickEvent(QMouseEvent *event) override;

        void onItemChanged(GraphicsItemsChangeNotifier* item, MapViewCustomItemsEventFlag flag) override;

    private slots:
        void _displayLoader();
        void _onUIAlterationRequest(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter);
        void _onUIUpdateRequest(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void _onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates);
        void _onUIUserChangeRequest(const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser);

    private:
        QBrush* _hiddingBrush = nullptr;
        MapHint* _hints = nullptr;
        AtomsContextualMenuHandler* _menuHandler = nullptr;
        static inline constexpr int _defaultSceneSize = 36000;
        
        void _handleHintsSignalsAndSlots();
        void _updateItemValue(QGraphicsItem* item, const AtomUpdates &updates);

        //helper
        void _addItem(QGraphicsItem* toAdd, bool mustNotifyMovement = false);
        void _centerItemToPoint(QGraphicsItem* item, const QPoint &eventPos);
        QTimer _debounceSelection;
        void _goToDefaultViewState();
        
        bool _isLoading = false;
        void _hideLoader();

        //ghost
            void _mightCenterGhostWithCursor();
        
        //network
            void _sendMapHistory();
            void _onIdentityReceived(const RPZUser &self);

        //registered points
            bool _isMousePressed = false;

        //tool
            Tool _tool = (Tool)0;
            Tool _quickTool = (Tool)0;
            Tool _getCurrentTool() const;
            void _changeTool(Tool newTool, bool quickChange = false);
            void _resetTool();
            
        //moving...
            void _goToSceneCenter();
            void _animatedMove(const AnimationTimeLine::Type &orientation, int correction);
            QSet<QGraphicsItem*> _itemsWhoNotifiedMovement;

        //focusing...
            void _focusItem(QGraphicsItem* toFocus);
            QRectF _getVisibleRect();

        //zooming...
            const double _defaultScale = 5;
            double _currentRelScale = 1;
            void _goToDefaultZoom();

        //drawing...
            MapViewGraphicsPathItem* _tempDrawing = nullptr;
            QList<QGraphicsItem*> _tempDrawingHelpers;
            bool _stickyBrushIsDrawing = false;
            int _stickyBrushValidNodeCount = 0;

            void _destroyTempDrawing();
            void _beginDrawing(const QPoint &lastPointMousePressed);
            void _endDrawing();
            void _updateDrawingPath(const QPoint &evtPoint);
            void _updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter, MapViewGraphicsPathItem* sourceTemplate);
            void _savePosAsStickyNode(const QPoint &evtPoint);
};