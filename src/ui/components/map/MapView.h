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
#include <QList>

#include "MapTools.h"
#include "AssetsNavigator.h"
#include "Asset.hpp"

#include "src/helpers/_serializer.hpp"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"

class MapView : public QGraphicsView {

    Q_OBJECT

    public:
        enum MapElementEvtState { Changed, Added, Removed, Selected, Focused };
        MapView(QWidget *parent);

        //network helpers...
        QVariantList packageForNetworkSend(QList<Asset> &assets, const MapView::MapElementEvtState &state);
        void unpackFromNetworkReceived(const QVariantList &package);

    public slots:
        void changeToolFromAction(QAction *action);
        void changePenSize(const int newSize);
        void alterScene(const QList<QUuid> &elementIds, const MapView::MapElementEvtState &state);
        
        //network
        void bindToRPZClient(RPZClient * cc);
    
    signals:
        void mapElementsAltered(const QList<Asset> &elements, const MapElementEvtState &state);
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
        RPZClient * _currentCC = nullptr;
        QGraphicsScene* _scene;
        bool _externalInstructionPending = false;
        void _onSceneSelectionChanged();

        //registered points
            QPoint _lastPointMousePressing;
            QPoint _lastPointMousePressed;
            bool _isMousePressed = false;
            bool _isMiddleToolLock = false;

        //elements of map
            QHash<QUuid, Asset> _assetsById;
            QHash<QGraphicsItem*, QUuid> _idsByGraphicItem;
            QSet<QUuid> _selfElements;
            QHash<QUuid, QSet<QUuid>> _foreignElementIdsByOwnerId;

            QHash<QUuid, Asset> _fetchAssetsWithIds(const QList<QGraphicsItem*> &listToFetch) const;
            QList<Asset> _fetchAssets(const QList<QGraphicsItem*> &listToFetch) const;
            QList<Asset> _fetchAssets(const QList<QUuid> &listToFetch) const;

            void _alterScene(const MapElementEvtState &alteration, const QList<QGraphicsItem*> &elements);
            void _alterScene(const MapElementEvtState &alteration, QList<Asset> &assets);
            void _alterScene(const MapElementEvtState &alteration, const QList<QUuid> &elementIds);
            void _alterScene(const MapElementEvtState &alteration, Asset &asset);
            QUuid _alterSceneInternal(const MapElementEvtState &alteration, Asset &asset);

        //tool
            MapTools::Actions _selectedTool;
            static const MapTools::Actions _defaultTool = MapTools::Actions::Select;
            MapTools::Actions _quickTool = MapTools::Actions::None;
            void _changeTool(MapTools::Actions newTool, const bool quickChange = false);
            MapTools::Actions _getCurrentTool() const;
            void _toolOnMousePress(const MapTools::Actions &tool);
            void _toolOnMouseRelease(const MapTools::Actions &tool);

        //rotating...
            QCursor * _rotateCursor = nullptr;
            double _degreesFromNorth = 0;
            void _rotate(const QPoint &evtPoint);
            void _rotateBackToNorth();

        //zooming...
            int _numScheduledScalings = 0;
            void _zoomBy(const qreal factor);
            void _zoomBy_scalingTime(const qreal x);
            void _zoomBy_animFinished();
        
        //droping..
            // QPoint* _latestPosDrop;

        //drawing...
            int _penWidth = 1;
            QColor _penColor = Qt::blue;
            QPen _getPen() const;

            QList<QGraphicsItem*> _tempLines;
            QPainterPath* _tempDrawing = nullptr;
            void _beginDrawing();
            void _endDrawing();
            void _drawLineTo(const QPoint &evtPoint);

};