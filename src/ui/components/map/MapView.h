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

    Q_OBJECT

    public:
        enum MapElementEvtState { Changed, Added, Removed };
        MapView(QWidget *parent);
        void changeToolFromAction(QAction *action);
        void changePenSize(int newSize);
    
    signals:
        void mapElementsAltered(QHash<QUuid, QGraphicsItem*> elements, MapElementEvtState state);
        void unselectCurrentToolAsked();

    protected:
        void wheelEvent(QWheelEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;

    private:
        QGraphicsScene* _scene;
        
        //registered points
            QPoint _lastPointMousePressing;
            QPoint _lastPointMousePressed;
            bool _isMousePressed = false;
            bool _isMiddleToolLock = false;
            // QPoint* _latestPosDrop;

        //elements of map
            QHash<QUuid, QGraphicsItem*> _elementById;
            QHash<QGraphicsItem*, QUuid> _IdByElement;
            QSet<QUuid> _selfElements;
            QHash<JSONSocket*, QSet<QUuid>> _elementsByOwner;

            void _alterElements(MapElementEvtState alteration, QList<QGraphicsItem*> elements, JSONSocket* owner = nullptr);
            void _alterElement(MapElementEvtState alteration, QGraphicsItem* element, JSONSocket* owner = nullptr);
            QUuid _alterElementInternal(MapElementEvtState alteration, QGraphicsItem* element, JSONSocket* owner);

        //framerate
            double _frameRate = 0;
            double _numFrames = 0;
            void _instFramerate();
            void _framerate_oneSecondTimeout();

        //tool
            MapTools::Actions _selectedTool;
            static const MapTools::Actions _defaultTool = MapTools::Actions::Select;
            MapTools::Actions _quickTool = MapTools::Actions::None;
            void _changeTool(MapTools::Actions newTool, bool quickChange = false);
            MapTools::Actions _getCurrentTool();
            void _toolOnMousePress(MapTools::Actions tool);
            void _toolOnMouseRelease(MapTools::Actions tool);

        //rotating...
            QCursor * _rotateCursor = nullptr;
            void _rotate(const QPoint &evtPoint);

        //zooming...
            int _numScheduledScalings = 0;
            void _zoomBy(qreal factor);
            void _zoomBy_scalingTime(qreal x);
            void _zoomBy_animFinished();

        //drawing...
            int _penWidth = 1;
            QColor _penColor = Qt::blue;
            QPen _getPen();

            QList<QGraphicsItem*> _tempLines;
            QPainterPath* _tempDrawing = nullptr;
            void _beginDrawing();
            void _endDrawing();
            void _drawLineTo(const QPoint &evtPoint);

};