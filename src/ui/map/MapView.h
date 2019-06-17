#pragma once

#include <QtMath>
#include <math.h>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QWidget>

#include <QWheelEvent>
#include <QTreeWidgetItem>
#include <QVariantHash>
#include <QTimeLine>
#include <QGLWidget>
#include <QCursor>
#include <QTimer>
#include <QScrollBar>
#include <QVariant>
#include <QVector>

#include "graphics/MapViewGraphicsScene.hpp"

#include "MapTools.h"
#include "base/AnimationTimeLine.hpp"

#include "src/shared/atoms/MapHint.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/others/ClientBindable.hpp"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"

#include "src/ui/assets/base/AssetsTreeView.h"

#include "src/shared/payloads/Payloads.h"

#include "src/shared/models/AssetMetadata.hpp"

class MapView : public QGraphicsView, public ClientBindable {

    Q_OBJECT

    public:
        enum Tool { Default, Atom, Scroll };

        MapView(QWidget *parent);
        MapHint* hints();

    public slots:
        void actionRequested(const MapTools::Actions &action);
        void assetTemplateChanged(const QVariantHash &assetMetadata);
        void onAtomTemplateChange();
        void onBrushToolChange(int brushTool, int brushToolWidth);
    
    signals:
        void unselectCurrentAssetAsked();
        void remoteChanged(bool isRemote);
        void subjectedAtomsChanged(QVector<void*> &subjectAtoms);

    protected:
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;

        void onRPZClientConnecting(RPZClient * cc) override;
        void onRPZClientDisconnect(RPZClient* cc) override;

        void contextMenuEvent(QContextMenuEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;
        void resizeEvent(QResizeEvent * event) override;

    private:
        MapViewGraphicsScene* _scene = nullptr;

        MapHint* _hints;
        void _onSceneSelectionChanged();
        void _goToDefaultViewState();

        //ghost
            QGraphicsItem* _ghostItem = nullptr;
            AssetMetadata _bufferedAssetMetadata;
            void _clearGhostItem();
            void _generateGhostItemFromBuffer();
            void _handleGhostItem(const Tool &tool);
        
        //network
            void _sendMapChanges(QVariantHash &payload);
            void _sendMapHistory();

        //registered points
            bool _isMousePressed = false;

        //tool
            BrushType _bTool = (BrushType)0;
            int _bToolWidth = 1;
            Tool _tool = (Tool)0;
            Tool _quickTool = (Tool)0;
            Tool _getCurrentTool() const;
            void _changeTool(Tool newTool, const bool quickChange = false);
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
            QGraphicsPathItem* _tempDrawing = nullptr;
            AtomType _currentDrawing_AtomType = (AtomType)0;

            void _destroyTempDrawing();
            void _beginDrawing(const QPoint &lastPointMousePressed);
            void _endDrawing();
            void _updateDrawingPath(const QPoint &evtPoint);
            void _updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter);

};