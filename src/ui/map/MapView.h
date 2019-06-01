#pragma once

#include <QtMath>
#include <math.h>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QWidget>

#include <QWheelEvent>
#include <QTreeWidgetItem>
#include <QVariantHash>
#include <QAction>
#include <QTimeLine>
#include <QGLWidget>
#include <QCursor>
#include <QTimer>
#include <QScrollBar>
#include <QUuid>
#include <QVariant>
#include <QVector>

#include "graphics/MapViewGraphicsScene.hpp"

#include "MapTools.h"
#include "base/AnimationTimeLine.hpp"

#include "src/shared/map/MapHintViewBinder.h"
#include "src/shared/models/entities/RPZAtom.hpp"
#include "src/ui/others/ClientBindable.hpp"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"

#include "src/ui/assets/base/AssetsTreeView.h"

#include "src/shared/models/Payloads.h"


class MapView : public QGraphicsView, public ClientBindable {

    Q_OBJECT

    public:
        MapView(QWidget *parent);
        MapHintViewBinder* hints();

    public slots:
        void changeToolFromAction(const MapTools::Actions &instruction);
        
        //network
        void onRPZClientConnecting(RPZClient * cc) override;
        void onRPZClientDisconnect(RPZClient* cc) override;
    
    signals:
        void unselectCurrentToolAsked();
        void remoteChanged(bool isRemote);

    protected:
        void leaveEvent(QEvent *event) override;
        void enterEvent(QEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void dropEvent(QDropEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragLeaveEvent(QDragLeaveEvent *event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;
        void resizeEvent(QResizeEvent * event) override;

    private:
        QGraphicsTextItem* _tempText = nullptr;
        void _clearTempText();

        MapViewGraphicsScene* _scene = nullptr;

        MapHintViewBinder* _hints;
        void _onSceneSelectionChanged();
        void _goToDefaultViewState();
        
        //network
            void _sendMapChanges(QVariantHash &payload);
            void _sendMapHistory();

        //registered points
            QPoint _lastPointMousePressing;
            QPoint _lastPointMousePressed;
            bool _isMousePressed = false;
            bool _isMiddleToolLock = false;

        //tool
            MapTools::Actions _selectedTool;
            static const MapTools::Actions _defaultTool = MapTools::Actions::Select;
            MapTools::Actions _quickTool = MapTools::Actions::None;
            void _changeTool(MapTools::Actions newTool, const bool quickChange = false);
            MapTools::Actions _getCurrentTool() const;
            void _toolOnMousePress(const MapTools::Actions &tool);
            void _toolOnMouseRelease(const MapTools::Actions &tool);

        //moving...
            const int _defaultSceneSize = 36000;
            void _goToSceneCenter();
            void _animatedMove(const Qt::Orientation &orientation, int correction);

        //rotating...
            QCursor * _rotateCursor = nullptr;
            double _degreesFromNorth = 0;
            void _rotateFromPoint(const QPoint &evtPoint);
            void _rotate(double deg);
            void _animatedRotation(double deg);
            void _rotateBackToNorth();

        //d&d
            AssetsDatabase* _droppableSourceDatabase = nullptr;
            AssetsDatabaseElement* _droppableElement = nullptr;
            QGraphicsItem* _droppableGraphicsItem = nullptr;

        //zooming...
            const double _defaultScale = 5;
            double _currentRelScale = 1;
            void _goToDefaultZoom();

        //drawing...
            QGraphicsPathItem* _tempDrawing = nullptr;
            void _beginDrawing();
            void _endDrawing();
            void _drawLineTo(const QPoint &evtPoint);

};