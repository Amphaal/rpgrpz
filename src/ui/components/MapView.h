#pragma once

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtMath>
#include <QWheelEvent>
#include <QGraphicsSvgItem>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QAction>
#include <QTimeLine>
#include <QGLWidget>
#include <QCursor>
#include <QTimer>
#include <QScrollBar>
#include <QUuid>

#include "MapTools.h"
#include "AssetsNavigator.h"
#include "src/network/rpz/_any/JSONSocket.h"

class MapView : public QGraphicsView {

    public:
        MapView(QWidget *parent);

        void toolSelectionChanged(QAction *action);
    
    protected:
        void wheelEvent(QWheelEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;

    private:
        QGraphicsScene* _scene;
        // QPoint* _latestPosDrop;
        QPoint _lastPointMousePressed;
        QPoint _lastPointMouseClick;
        bool _isMousePressed = false;

        //framerate
        double _frameRate = 0;
        double _numFrames = 0;
        void _instFramerate();
        void _framerate_oneSecondTimeout();

        //tool
        static const MapTools::Actions _defaultTool = MapTools::Actions::Select;
        MapTools::Actions _selectedTool = MapView::_defaultTool;
        MapTools::Actions _quickTool = MapTools::Actions::None;
        void _changeTool(MapTools::Actions newTool, bool quickChange = false);
        MapTools::Actions _getCurrentTool();

        //rotating...
        QCursor * _rotateCursor = nullptr;
        void _rotate(const QPoint &evtPoint);

        //scrolling...
        void _scroll(const QPoint &evtPoint);

        //zooming...
        int _numScheduledScalings = 0;
        void _zoomBy(qreal factor);
        void _zoomBy_scalingTime(qreal x);
        void _zoomBy_animFinished();

        //drawing...
        QMap<QUuid, QGraphicsPathItem*> _selfDrawings;
        // QMap<JSONSocket*, QMap<QUuid, QGraphicsPathItem*>> _drawingsByUsers;
        QList<QGraphicsItem*> _tempLines;
        QPainterPath* _tempDrawing = nullptr;
        void _drawLineTo(const QPoint &evtPoint);
        int _penWidth = 1;
        QColor _penColor = Qt::blue;
        QPen _getPen();

};