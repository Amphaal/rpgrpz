// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include "DocShareListView.h"

DocShareListView::DocShareListView(QWidget *parent) : QListWidget(parent) {
    this->setAcceptDrops(true);

    QObject::connect(
        this, &QListWidget::itemDoubleClicked,
        this, &DocShareListView::_onItemDoubleClick
    );
}

void DocShareListView::connectingToServer() {
    QObject::connect(
        this->_rpzClient, &RPZClient::gameSessionReceived,
        this, &DocShareListView::_onGameSessionReceived
    );

    QObject::connect(
        this->_rpzClient, &RPZClient::sharedDocumentAvailable,
        this, &DocShareListView::_mayAddTemporaryItem
    );

    QObject::connect(
        this->_rpzClient, &RPZClient::sharedDocumentReceived,
        this, &DocShareListView::_updateItemFromNetwork
    );
}
void DocShareListView::connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) {
    // allow drops
    _allowDrop = true;

    QList<RPZSharedDocument::FileHash> hashesToDelete;

    // iterate
    for (auto i = this->_itemByHash.begin(); i != this->_itemByHash.end(); i++) {
        auto &hash = i.key();
        auto &item = i.value();

        // add for removal if in a non-local state
        auto state = (DownloadState)item->data(DocShareListView::DownloadStateRole).toInt();
        if (state == Downloading || state == Downloadable) hashesToDelete += hash;
    }

    // remove from registry dans from view
    for (auto &hash : hashesToDelete) {
        auto item = this->_itemByHash.take(hash);
        delete item;
    }
}

Qt::DropActions DocShareListView::supportedDropActions() const {
    return (Qt::DropAction::MoveAction | Qt::DropAction::CopyAction);
}

void DocShareListView::_onGameSessionReceived(const RPZGameSession &gs) {
    // prevent drop for non hosts
    _allowDrop = Authorisations::isHostAble();

    // prevent shared documents autoload if host
    if (_allowDrop) return;

    // iterate through shared docs
    auto namesStore = gs.sharedDocumentsNS();
    for (auto i = namesStore.begin(); i != namesStore.end(); i++) {
        auto &hash = i.key();
        auto &name = i.value();

        // add them
        this->_mayAddTemporaryItem(hash, name);
    }
}

void DocShareListView::_onItemDoubleClick(QListWidgetItem *item) {
    // check if downloading
    auto state = (DownloadState)item->data(DocShareListView::DownloadStateRole).toInt();
    if (state == DownloadState::Downloading) return;

    // try to open associated file
    auto pathToFile = item->data(DocShareListView::FilePathRole).toString();
    if (!pathToFile.isEmpty()) return AppContext::openFileInOS(pathToFile);

    // file path is empty, check client exists
    if (!this->_rpzClient) return;

    // get data
    auto hash = item->data(DocShareListView::HashRole).toString();
    auto fileName = item->data(DocShareListView::DocumentNameRole).toString();

    // update state
    item->setData(DocShareListView::DownloadStateRole, DownloadState::Downloading);
    item->setText(fileName + QObject::tr(" (downloading...)"));

    // ask for file
    QMetaObject::invokeMethod(this->_rpzClient, "requestSharedDocument", Q_ARG(QString, hash));
}

void DocShareListView::_mayStoreAsDocument(const QUrl &fileUrl) {
    // try to create a shared document
    RPZSharedDocument doc(fileUrl);
    if (!doc.isSuccess()) return;

    // override file if exists
    auto hash = doc.documentFileHash();
    auto alreadyExists = SharedDocHint::containsHash(hash);

    if (alreadyExists) return;
    this->_addItem(doc);
}

void DocShareListView::_updateItemFromNetwork(const RPZSharedDocument &doc) {
    auto hash = doc.documentFileHash();

    // update item state
    auto item = this->_itemByHash.value(hash);
    this->_updateItem(doc, item);
}

void DocShareListView::_updateItem(const RPZSharedDocument &doc, QListWidgetItem* item) {
    auto temporaryFilePath = doc.writeAsTemporaryFile();
    auto docName = doc.documentName();

    item->setData(DocShareListView::FilePathRole, temporaryFilePath);
    item->setData(DocShareListView::DownloadStateRole, DownloadState::Downloaded);
    item->setData(DocShareListView::DocumentNameRole, docName);

    // icon
    QFileInfo tempFi(temporaryFilePath);
    auto icon = this->_iconProvider.icon(tempFi);
    item->setIcon(icon);
    item->setText(docName);

    SharedDocHint::insertIntoStore(doc);
}

void DocShareListView::_addItem(const RPZSharedDocument &doc) {
    // create item
    auto playlistItem = new QListWidgetItem;
    auto hash = doc.documentFileHash();
    playlistItem->setData(DocShareListView::HashRole, hash);

    // update item
    this->_updateItem(doc, playlistItem);

    // add it
    this->addItem(playlistItem);
    auto filename = playlistItem->text();

    //
    if (this->_rpzClient && Authorisations::isHostAble()) {
        QMetaObject::invokeMethod(this->_rpzClient, "addSharedDocument",
            Q_ARG(RPZSharedDocument::FileHash, hash),
            Q_ARG(RPZSharedDocument::DocumentName, filename)
        );
    }

    // add to registry
    this->_itemByHash.insert(hash, playlistItem);
}

void DocShareListView::_mayAddTemporaryItem(const RPZSharedDocument::FileHash &hash, const QString &fileName) {
    if (SharedDocHint::containsHash(hash)) return;

    auto playlistItem = new QListWidgetItem;

    // default icon
    auto icon = this->_iconProvider.icon(QFileInfo());
    playlistItem->setIcon(icon);

    playlistItem->setData(DocShareListView::HashRole, hash);
    playlistItem->setData(DocShareListView::DownloadStateRole, DownloadState::Downloadable);
    playlistItem->setData(DocShareListView::DocumentNameRole, fileName);

    playlistItem->setText(fileName + QObject::tr(" (available)"));

    // add to register
    this->_itemByHash.insert(hash, playlistItem);

    // update names store
    SharedDocHint::updateNamesStore(hash, fileName);

    // add to view
    this->addItem(playlistItem);
}

void DocShareListView::dragEnterEvent(QDragEnterEvent *event) {
    QWidget::dragEnterEvent(event);

    // if dragged from OS
    if (!event->mimeData()->hasUrls()) return;

    // create list of handled urls
    event->setDropAction(Qt::DropAction::MoveAction);
    event->acceptProposedAction();
}

void DocShareListView::dragMoveEvent(QDragMoveEvent * event)  {
    if(_allowDrop) event->accept();
    else event->ignore();
}

void DocShareListView::dropEvent(QDropEvent *event) {
    QListWidget::dropEvent(event);

    // for each link registered
    for (const auto &url : event->mimeData()->urls()) {
        _mayStoreAsDocument(url);
    }
}
