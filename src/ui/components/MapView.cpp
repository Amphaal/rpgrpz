#pragma once

#include <QWidget>
#include <QtOpenGL/QGLWidget>
#include <QGraphicsScene>
#include <QGraphicsView>

class MapView : public QGraphicsView {

    public:
        MapView(QWidget *parent)
        : QGraphicsView(parent), _scene(new QGraphicsScene) {
            
            this->setMinimumSize(400,400);

            auto background = new QBrush("#EEE", Qt::CrossPattern);

            this->setBackgroundBrush(*background);

            //auto rect = this->_scene->addRect(QRectF(0, 0, 100, 100));
            this->setScene(this->_scene);

        }

    private:
        QGraphicsScene* _scene;
};