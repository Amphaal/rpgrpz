#pragma once

#include <QTreeWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QString>
#include <QDebug>
#include <QHeaderView>
#include <QLayout>

#include "RPZTree.hpp"

class AssetsNavigator : public RPZTree {
    public:
        AssetsNavigator(QWidget * parent = nullptr);
    
    private:
        //drag and drop
        QMimeDatabase* _MIMEDb;
        void dropEvent(QDropEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;
        Qt::DropActions supportedDropActions() const override;
};