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

#include "src/shared/atoms/ViewMapHint.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/others/ClientBindable.hpp"

#include "src/network/rpz/_any/JSONSocket.h"
#include "src/network/rpz/client/RPZClient.h"

#include "src/ui/assets/base/AssetsTreeView.h"

#include "src/shared/payloads/Payloads.h"


class MapView : public QGraphicsView, public ClientBindable {

    Q_OBJECT

    public:
        MapView(QWidget *parent);
        ViewMapHint* hints();

    public slots:
        void changeToolFromAction(const MapTools::Actions &instruction);
        void useAssetTemplate(const AtomType &type, 
                const QString assetId = NULL, 
                const QString assetName = NULL, 
                const QString assetLocation = NULL);
        void updateGhostItemFromAtomTemplate(void* atomTemplate);
    
    signals:
        void unselectCurrentToolAsked();
        void unselectCurrentAssetAsked();
        void remoteChanged(bool isRemote);

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

        ViewMapHint* _hints;
        void _onSceneSelectionChanged();
        void _goToDefaultViewState();

        //ghost
            QGraphicsItem* _ghostItem = nullptr;
            bool _isGhostFrozen = false;
            QString _bufferedAssetLocation;
            void _clearGhostItem();
            void _generateGhostItem(const AtomType &type, 
                const QString assetId = NULL, 
                const QString assetName = NULL, 
                const QString assetLocation = NULL
            );
            void _generateGhostItem(const MapTools::Actions &action);
        
        //network
            void _sendMapChanges(QVariantHash &payload);
            void _sendMapHistory();

        //registered points
            bool _isMousePressed = false;
            bool _isMiddleToolLock = false;

        //tool
            static const MapTools::Actions _defaultTool = MapTools::Actions::None;
            MapTools::Actions _quickTool = MapTools::Actions::None;
            MapTools::Actions _selectedTool;
            MapTools::Actions _getCurrentTool() const;
            void _changeTool(MapTools::Actions newTool, const bool quickChange = false);
            
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
            void _beginDrawing(const QPoint &lastPointMousePressed);
            void _endDrawing();
            void _drawLineTo(const QPoint &evtPoint);

};