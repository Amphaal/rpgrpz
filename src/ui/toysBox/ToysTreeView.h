// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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

#include "src/ui/toysBox/_base/ToysTreeViewModel.h"
#include "src/shared/commands/RPZActions.h"

#include "src/shared/models/toy/RPZToy.hpp"

#include "src/ui/_others/ConnectivityObserver.h"
#include "src/shared/async-ui/AlterationHandler.h"

class ToysTreeView : public QTreeView, public ConnectivityObserver, public AlterationInteractor {

    Q_OBJECT

    public:
        ToysTreeView(QWidget *parent = nullptr);

    private slots:
        void _onAssetsAboutToBeDownloaded(const QVector<QString> &availableIds);
        void _onReceivedAsset(RPZAssetImportPackage package);
        void _handleAlterationRequest(const AlterationPayload &payload);

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;
        void keyPressEvent(QKeyEvent * event) override;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void startDrag(Qt::DropActions supportedActions) override;

    private:
        QModelIndexList _selectedElementsIndexes();

        int _expectedAssetsTBDownloaded = 0;
        int _expectedAssetsDownloaded = 0;

        void connectingToServer() override;

        QMimeDatabase _MIMEDb;
        ToysTreeViewModel* _model = nullptr;

        /////////////////////
        // Contextual menu //
        /////////////////////

        void _generateMenu(const QList<QModelIndex> &targetIndexes, const QPoint &whereToDisplay);

        QList<QAction*> _staticContainerMoveActions;
        void _generateStaticContainerMoveActions();

        /////////////////////////
        // END Contextual menu //
        /////////////////////////

        //auto expand on row insert
        void _onRowInsert(const QModelIndex &parent, int first, int last);
        void _requestDeletion(const QModelIndexList &itemsIndexesToDelete);

        RPZToy _selectedToy;

};