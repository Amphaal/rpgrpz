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


class AssetsNavigator : public QTreeWidget {
    public:
        AssetsNavigator(QWidget * parent = nullptr);
    
    private:
        QMimeDatabase* _MIMEDb;
        void dropEvent(QDropEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;
        Qt::DropActions supportedDropActions() const;
};