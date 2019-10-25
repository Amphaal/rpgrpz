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

#include "src/ui/toys/_base/ToysTreeViewModel.h"
#include "src/shared/commands/RPZActions.h"

#include "src/shared/models/toy/RPZToy.hpp"

class ToysTreeView : public QTreeView, public ConnectivityObserver, public AlterationActor {

    Q_OBJECT

    public:
        ToysTreeView(QWidget *parent = nullptr);
        ToysTreeViewModel* assetsModel();
        QModelIndexList selectedElementsIndexes();

    private slots:
        void _onAssetsAboutToBeDownloaded(const QVector<QString> &availableIds);
        void _onReceivedAsset(RPZAssetImportPackage package);
        void _handleAlterationRequest(const AlterationPayload &payload);

    private:
        int _expectedAssetsTBDownloaded = 0;
        int _expectedAssetsDownloaded = 0;

        void connectingToServer() override;

        ///////////////////
        // drag and drop //
        ///////////////////

        QMimeDatabase* _MIMEDb = nullptr;
        ToysTreeViewModel* _model = nullptr;

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

        RPZToy _selectedToy;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
};