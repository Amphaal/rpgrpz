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

#include "src/ui/map/graphics/CustomGraphicsItemHelper.h"

#include "MapTools.hpp"
#include "src/ui/map/_base/AnimationTimeLine.hpp"

#include "src/shared/hints/MapHint.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/_others/ConnectivityObserver.h"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"

#include "src/ui/toys/ToysTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/helpers/AtomConverter.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

#include "src/ui/map/modules/MV_Manipulation.hpp"
#include "src/ui/map/modules/MV_HUDLayout.hpp"

#include "src/ui/map/assists/DrawingAssist.hpp"

class MapView : public QGraphicsView, public ConnectivityObserver, public MV_Manipulation, public MV_HUDLayout {

    Q_OBJECT

    public:
        MapView(QWidget *parent = nullptr);
        ~MapView();

        MapHint* hints() const;

    public slots:
        void onActionRequested(const MapAction &action);
        void onHelperActionTriggered(QAction *action);
    
    signals:
        void remoteChanged(bool isRemote);

    protected:
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;

        void connectingToServer() override;
        void connectionClosed() override;

        void contextMenuEvent(QContextMenuEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;
        void keyReleaseEvent(QKeyEvent *event) override;
        void resizeEvent(QResizeEvent * event) override;
        void mouseDoubleClickEvent(QMouseEvent *event) override;

        void drawBackground(QPainter *painter, const QRectF &rect) override;
        void drawForeground(QPainter *painter, const QRectF &rect) override;

    private slots:
        void _onUIAlterationRequest(const Payload::Alteration &type, const QList<QGraphicsItem*> &toAlter);
        void _onUIUpdateRequest(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate);
        void _onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates);

        //network
        void _sendMapHistory();
        void _onIdentityReceived(const RPZUser &self);

    private:
        DrawingAssist* _drawingAssist = nullptr;
        MapHint* _hints = nullptr;
        AtomsContextualMenuHandler* _menuHandler = nullptr;
        static inline constexpr int _defaultSceneSize = 36000;
        
        void _handleHintsSignalsAndSlots();
        void _updateItemValue(QGraphicsItem* item, const AtomUpdates &updates);

        //selection...
        QTimer _selectionDebouncer;

        //ghost
            void _mightCenterGhostWithCursor();

        //registered points
            bool _isMousePressed = false;

        //tool
            MapTool _tool = MapTool::Default;
            MapTool _quickTool = MapTool::Default;
            MapTool _getCurrentTool() const;
            void _changeTool(MapTool newTool, bool quickChange = false);
            void _resetTool();
        
        void onAnimationManipulationTickDone() override;
};