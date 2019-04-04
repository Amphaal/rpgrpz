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

#include "MapTools.h"
#include "AssetsNavigator.h"

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
        QPoint* _latestPosDrop;

        //framerate
        double _frameRate = 0;
        double _numFrames = 0;
        void _instFramerate();
        void _framerate_oneSecondTimeout();

        //tool
        QPointF _lastPoint;
        static const MapTools::Actions _defaultTool = MapTools::Actions::Select;
        MapTools::Actions _selectedTool = MapView::_defaultTool;
        MapTools::Actions _quickTool = MapTools::Actions::None;
        void _changeTool(MapTools::Actions newTool, bool quickChange = false);
        MapTools::Actions _getCurrentTool();

        //rotating...
        QCursor * _rotateCursor = nullptr;
        void _rotate(const QPointF &endPoint);

        //moving...

        //zooming...
        int _numScheduledScalings = 0;
        void _zoomBy(qreal factor);
        void _zoomBy_scalingTime(qreal x);
        void _zoomBy_animFinished();

        //drawing...
        int _penWidth = 1;
        QColor _penColor = Qt::blue;
        void _drawLineTo(const QPointF &endPoint);


};