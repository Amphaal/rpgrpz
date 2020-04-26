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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QListWidget>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QFileIconProvider>

#include "src/ui/_others/ConnectivityObserver.h"
#include "src/shared/models/RPZSharedDocument.hpp"
#include "src/shared/hints/SharedDocHint.hpp"

class DocShareListView : public QListWidget, public ConnectivityObserver {
    Q_OBJECT

 public:
    explicit DocShareListView(QWidget *parent = nullptr);

 protected:
    void connectingToServer() override;
    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override;

 private slots:
    void _onGameSessionReceived(const RPZGameSession &gs);
    void _updateItemFromNetwork(const RPZSharedDocument &doc);
    void _mayAddTemporaryItem(const RPZSharedDocument::FileHash &hash, const QString &fileName);

 private:
    QHash<RPZSharedDocument::FileHash, QListWidgetItem*> _itemByHash;

    QMimeDatabase _MIMEDb;
    QFileIconProvider _iconProvider;
    bool _allowDrop = true;

    static inline int HashRole = 3000;
    static inline int FilePathRole = 3100;
    static inline int DownloadStateRole = 3200;
    static inline int DocumentNameRole = 3300;

    enum DownloadState {
        Downloaded,
        Downloading,
        Downloadable
    };

    void _onItemDoubleClick(QListWidgetItem *item);

    void _mayStoreAsDocument(const QUrl &fileUrl);
    void _updateItem(const RPZSharedDocument &doc, QListWidgetItem* item);
    void _addItem(const RPZSharedDocument &doc);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dropEvent(QDropEvent *event) override;
    Qt::DropActions supportedDropActions() const override;
};
