#pragma once

#include <math.h>

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
#include <QVariant>
#include <QVector>

#include "MapTools.h"
#include "base/AssetsNavigator.h"
#include "base/AnimationTimeLine.hpp"

#include "src/shared/map/MapHintViewBinder.h"
#include "src/shared/RPZAsset.hpp"
#include "src/shared/ClientBindable.hpp"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"


class MapView : public QGraphicsView, public ClientBindable {

    Q_OBJECT

    public:
        MapView(QWidget *parent);
        MapHintViewBinder* hints();

    public slots:
        void changeToolFromAction(const MapTools::Actions &instruction);
        void changePenSize(const int newSize);
        
        //network
        void bindToRPZClient(RPZClient * cc) override;
    
    signals:
        void unselectCurrentToolAsked();
        void assetsAlteredForLocal(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements);
        void assetsAlteredForNetwork(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements);

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
        MapHintViewBinder* _hints;
        void _onSceneSelectionChanged();
        void _goToDefaultViewState();
        
        //network
            void _sendMapChanges(const RPZAsset::Alteration &state, QVector<RPZAsset> &elements);
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
            int _numScheduledMovesVertical = 0;
            int _numScheduledMovesHorizontal = 0;
            const int _defaultSceneSize = 36000;
            void _goToSceneCenter();
            void _animatedMove(const Qt::Orientation &orientation, int correction);

        //rotating...
            int _numScheduledRotations = 0;
            QCursor * _rotateCursor = nullptr;
            double _degreesFromNorth = 0;
            void _rotateFromPoint(const QPoint &evtPoint);
            void _rotate(double deg);
            void _animatedRotation(double deg);
            void _rotateBackToNorth();

        //zooming...
            const double _defaultScale = 5;
            int _numScheduledScalings = 0;
            double _currentRelScale = 1;
            void _goToDefaultZoom();
            void _zoomBy(const qreal factor);
            void _zoomBy_scalingTime(int* stateController, const qreal x);
        
        //droping..
            // QPoint* _latestPosDrop;

        //drawing...
            int _penWidth = 1;
            QColor _penColor = Qt::blue;
            QPen _getPen() const;

            QVector<QGraphicsItem*> _tempLines;
            QPainterPath* _tempDrawing = nullptr;
            void _beginDrawing();
            void _endDrawing();
            void _drawLineTo(const QPoint &evtPoint);

};