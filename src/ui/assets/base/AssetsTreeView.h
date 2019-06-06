#pragma once

#include <QMimeDatabase>
#include <QTreeView>
#include <QHeaderView>
#include <QMessageBox>
#include <QDrag>
#include <QPainter>

#include <QMenu>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include "AssetsTreeViewModel.h"

class AssetsTreeView : public QTreeView {

    Q_OBJECT

    public:
        AssetsTreeView(QWidget *parent = nullptr);
        AssetsTreeViewModel* assetsModel();
        QModelIndexList selectedElementsIndexes();
    
    signals:
        void requestAssetPreview(QString assetLocation);
        void requestPreviewReset();

    private:

        void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        ///////////////////
        // drag and drop //
        ///////////////////

        QMimeDatabase* _MIMEDb = nullptr;
        AssetsTreeViewModel* _model = nullptr;

        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void startDrag(Qt::DropActions supportedActions) override;

        ///////////////////////
        // END drag and drop //
        ///////////////////////

        /////////////////////
        // Contextual menu //
        /////////////////////

        void _renderCustomContextMenu(const QPoint &pos);
        void _generateMenu(QList<QModelIndex> &itemsIndexes, const QPoint &whereToDisplay);

        /////////////////////////
        // END Contextual menu //
        /////////////////////////

        //auto expand on row insert
        void _onRowInsert(const QModelIndex &parent, int first, int last);
        void _requestDeletion(const QModelIndexList &itemsIndexesToDelete);

        void keyPressEvent(QKeyEvent * event) override;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
};