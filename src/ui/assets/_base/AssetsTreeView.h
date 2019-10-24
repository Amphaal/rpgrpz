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
#include "src/shared/commands/RPZActions.h"

#include "src/shared/models/toy/RPZAsset.hpp"

class AssetsTreeView : public QTreeView, public ConnectivityObserver, public AlterationActor {

    Q_OBJECT

    public:
        AssetsTreeView(QWidget *parent = nullptr);
        AssetsTreeViewModel* assetsModel();
        QModelIndexList selectedElementsIndexes();

    private slots:
        void _onAssetsAboutToBeDownloaded(const QVector<QString> &availableIds);
        void _onReceivedAsset(const RPZAssetImportPackage &package);
        void _handleAlterationRequest(const AlterationPayload &payload);

    private:
        int _expectedAssetsTBDownloaded = 0;
        int _expectedAssetsDownloaded = 0;

        void connectingToServer() override;

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
        void _generateMenu(const QList<QModelIndex> &targetIndexes, const QPoint &whereToDisplay);

        QList<QAction*> _staticContainerMoveActions;
        void _generateStaticContainerMoveActions();

        /////////////////////////
        // END Contextual menu //
        /////////////////////////

        //auto expand on row insert
        void _onRowInsert(const QModelIndex &parent, int first, int last);
        void _requestDeletion(const QModelIndexList &itemsIndexesToDelete);

        void keyPressEvent(QKeyEvent * event) override;

        RPZAsset _selectedAsset;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
};