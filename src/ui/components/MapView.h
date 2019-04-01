#pragma once

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtMath>
#include <QWheelEvent>
#include <QGraphicsSvgItem>

class MapView : public QGraphicsView {

    public:
        MapView(QWidget *parent);
        qreal zoomFactor() const;
    
    protected:
        void wheelEvent(QWheelEvent *event) override;

    private:
        QGraphicsScene* _scene;

        void zoomBy(qreal factor);
};