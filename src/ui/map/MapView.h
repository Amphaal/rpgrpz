#pragma once

#include <QtMath>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QWidget>

#include <QWheelEvent>
#include <QTreeWidgetItem>
#include <QVariantHash>
#include <QTimeLine>
#include <QGLWidget>
#include <QCursor>
#include <QScrollBar>
#include <QVariant>
#include <QVector>

#include "graphics/MapViewGraphicsScene.h"

#include "MapTools.h"
#include "base/AnimationTimeLine.hpp"

#include "src/shared/atoms/MapHint.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/others/ClientBindable.h"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClientThread.h"

#include "src/ui/assets/base/AssetsTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/shared/models/RPZAssetMetadata.h"

class MapView : public QGraphicsView, public ClientBindable {

    Q_OBJECT

    public:
        enum Tool { Default, Atom, Scroll };

        MapView(QWidget *parent);
        MapHint* hints() const;

    public slots:
        void actionRequested(const MapTools::Actions &action);
        void assetTemplateChanged(const QVariantHash &assetMetadata);
        void onAtomTemplateChange();
    
    signals:
        void unselectCurrentAssetAsked();
        void remoteChanged(bool isRemote);
        void subjectedAtomsChanged(QVector<RPZAtom*> &subjectAtoms);

    protected:
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;

        void onRPZClientThreadConnecting() override;
        void onRPZClientThreadDisconnect() override;

        void contextMenuEvent(QContextMenuEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;
        void resizeEvent(QResizeEvent * event) override;

    private slots:

    
    private:
        void _handleHintsSignalsAndSlots();

        //scene
            MapViewGraphicsScene* _scene = nullptr;
            QBrush* _hiddingBrush = nullptr;

        MapHint* _hints = nullptr;
        void _onSceneSelectionChanged();
        void _goToDefaultViewState();

        //helper
        void _centerItemToPoint(QGraphicsItem* item, const QPoint &eventPos);

        //ghost
            QGraphicsItem* _ghostItem = nullptr;
            RPZAssetMetadata _bufferedAssetMetadata;
            void _clearGhostItem();
            void _generateGhostItemFromBuffer();
            void _handleGhostItem(const Tool &tool);
        
        //network
            void _sendMapHistory();
            void _onIdentityReceived(const QVariantHash &userHash);

        //registered points
            bool _isMousePressed = false;

        //tool
            Tool _tool = (Tool)0;
            Tool _quickTool = (Tool)0;
            Tool _getCurrentTool() const;
            void _changeTool(Tool newTool, bool quickChange = false, bool isFromExternal = false);
            void _resetTool();
            
        //moving...
            const int _defaultSceneSize = 36000;
            void _goToSceneCenter();
            void _animatedMove(const Qt::Orientation &orientation, int correction);

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