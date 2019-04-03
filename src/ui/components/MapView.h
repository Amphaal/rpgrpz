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
#include <QGLWidget>

#include "MapTools.h"
#include "AssetsNavigator.h"

class MapView : public QGraphicsView {

    public:
        MapView(QWidget *parent);
        qreal zoomFactor() const;

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

        bool _selectedTool = MapTools::Actions::Draw;

        void _zoomBy(qreal factor);

        QPointF _lastPoint;
        int _penWidth = 1;
        QColor _penColor = Qt::blue;
        void _drawLineTo(const QPointF &endPoint);
};