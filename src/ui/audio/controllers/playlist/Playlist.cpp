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

#include "Playlist.h"

Playlist::Playlist(QWidget* parent) : QListWidget(parent),
    _ytIconGrey(new QIcon(QStringLiteral(u":/icons/app/audio/youtubeGrey.png"))),
    _ytIcon(new QIcon(QStringLiteral(u":/icons/app/audio/youtube.png"))),
    _ytIconErr(new QIcon(QStringLiteral(u":/icons/app/audio/youtubeError.png"))) {
    // self
    this->setAcceptDrops(true);
    this->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    QObject::connect(
        this, &QListWidget::itemDoubleClicked,
        this, &Playlist::_onItemDoubleClicked
    );

    // add from db
    for (const auto &ytId : PlaylistDatabase::get()->ytIds()) {
        this->_addYoutubeVideo(ytId);
    }
}

Qt::DropActions Playlist::supportedDropActions() const {
    return (Qt::DropAction::MoveAction | Qt::DropAction::CopyAction);
}

void Playlist::dragEnterEvent(QDragEnterEvent *event) {
    QWidget::dragEnterEvent(event);

    // if dragged from OS
    if (event->mimeData()->hasUrls()) {
        // create list of handled urls
        auto urls = event->mimeData()->urls();
        auto handledUrlsCount = this->_tempHashDnDFromUrlList(urls);

        // if there is a single handledUrls, continue
        if (handledUrlsCount) {
            event->setDropAction(Qt::DropAction::MoveAction);
            event->acceptProposedAction();
        }
    }
}

int Playlist::_tempHashDnDFromUrlList(QList<QUrl> &list) {
    // clear temp content
    this->_tempDnD.clear();

    // iterate through
    for (const auto &url : list) {
        // if is not local file
        if (!url.isValid()) continue;

        // for local files...
        if (url.host().contains(QStringLiteral(u"youtu"))) {
            // check if video or playlist link
            auto query = QUrlQuery(url);

            // playlist
            if (query.hasQueryItem("list")) {
                this->_tempDnD.append(QPair(YoutubeUrlType::YoutubePlaylist, url));  // add...
            } else if (query.hasQueryItem("v")) {  // video
                // add...
                this->_tempDnD.append(QPair(YoutubeUrlType::YoutubeVideo, url));
            } else {  // unhandled format
                continue;
            }
        }
    }

    // return valid urls count
    return this->_tempDnD.size();
}

void Playlist::dragMoveEvent(QDragMoveEvent * event) {
    event->accept();
}

void Playlist::dropEvent(QDropEvent *event) {
    QListWidget::dropEvent(event);

    // for each link registered
    for (const auto &[type, url] : this->_tempDnD) {
        // defines behavior depending on tag
        switch (type) {
            case YoutubeUrlType::YoutubePlaylist: {
                // fetch videos from playlist
                AudioTube::NetworkFetcher::fromPlaylistUrl(url.toString())
                .then([=](const QList<AudioTube::VideoMetadata*> &mvideoList) {
                    for (const auto mvideo : mvideoList) {
                        this->addYoutubeVideo(mvideo->url());
                    }
                });
            }
            break;

            case YoutubeUrlType::YoutubeVideo: {
                this->addYoutubeVideo(url.toString());
            }
            break;
        }
    }

    // clear temp content
    this->_tempDnD.clear();
}

void Playlist::_removeYoutubeVideo(QListWidgetItem* playlistItem) {
    auto metadata = RPZQVariant::ytVideoMetadata(playlistItem);

    // remove from local list
    this->_playlistVideoIds.remove(metadata->id());

    // remove from view
    delete playlistItem;

    // remove from db
    PlaylistDatabase::get()->removeYoutubeId(metadata->id());

    // release from memory
    delete metadata;
}

void Playlist::keyPressEvent(QKeyEvent * event) {
    // switch
    switch (event->key()) {
        case Qt::Key::Key_Delete: {
            this->_deleteSelectedTracks();
        }
        break;

        case Qt::Key::Key_Escape: {
            this->clearSelection();
        }
        break;

        default:
            break;
    }

    QListWidget::keyPressEvent(event);
}

void Playlist::_deleteSelectedTracks() {
    if (!_askTrackDeletion()) return;
    for (auto selected : this->selectedItems()) {
        this->_removeYoutubeVideo(selected);
    }
}

void Playlist::addYoutubeVideo(const QString &url) {
    AudioTube::VideoMetadata* metadata = nullptr;

    // metadata definition
    try {
        metadata = AudioTube::VideoMetadata::fromVideoUrl(url);

        auto success = _addYoutubeItem(metadata);
        if (!success) {
            delete metadata;
            QToolTip::showText(this->mapToGlobal(QPoint()), tr("This Youtube video is already in playlist !"));
            return;
        }

        PlaylistDatabase::get()->addYoutubeId(metadata->id());
    }
    catch(...) {
        qWarning() << qUtf8Printable(QStringLiteral(u"Youtube Playlist : %1 is not a valid Youtube URL").arg(url));
        delete metadata;
        return;
    }
}

void Playlist::_addYoutubeVideo(const AudioTube::PlayerConfig::VideoId &ytVideoId) {
    auto metadata = AudioTube::VideoMetadata::fromVideoId(ytVideoId);
    auto success = _addYoutubeItem(metadata);
    if (!success) delete metadata;
}

bool Playlist::_addYoutubeItem(AudioTube::VideoMetadata* metadata) {
    auto url = metadata->url();
    auto id = metadata->id();
    auto title = PlaylistDatabase::get()->trackName(id);
    if (title.isEmpty()) title = url;

    // handle duplicates
    if (this->_playlistVideoIds.contains(id)) {
        return false;
    }
    this->_playlistVideoIds.insert(id);

    // prepare item
    auto playlistItem = new QListWidgetItem(title);

    // define inner data
    RPZQVariant::setYTVideoMetadata(playlistItem, metadata);

    // define default icon
    playlistItem->setIcon(*this->_ytIconGrey);

    // update text from playlist update
    QObject::connect(
        metadata, &AudioTube::VideoMetadata::metadataRefreshed,
        [=]() {
            auto durationStr = StringHelper::secondsToTrackDuration(metadata->playerConfig().duration());

            auto metadataTitle = metadata->playerConfig().title();
            auto completeTitle = QStringLiteral(u"%1 [%2]")
                            .arg(metadataTitle)
                            .arg(durationStr);

            playlistItem->setText(completeTitle);
            PlaylistDatabase::get()->setTrackName(id, metadataTitle);

            // define active YT icon
            playlistItem->setIcon(*this->_ytIcon);
    });

    QObject::connect(
        metadata, &AudioTube::VideoMetadata::metadataFetching,
        [=]() {
            playlistItem->setIcon(*this->_ytIconGrey);
            playlistItem->setText(tr("(Loading metadata...) ") + title);
    });

    QObject::connect(
        metadata, &AudioTube::VideoMetadata::streamFailed,
        [=]() {
            // add delay for user ack
            QTimer::singleShot(100, [=]() {
                playlistItem->setIcon(*this->_ytIconErr);
                playlistItem->setText(tr("(Error) ") + url);
            });
    });

    this->addItem(playlistItem);

    return true;
}


void Playlist::playNext() {
    if (this->_defaultPlay()) return;  // if default play is used, do nothing

    // find next elem
    auto selected_index = this->indexFromItem(this->_playlistItemToUse);
    auto next_index = selected_index.siblingAtRow(selected_index.row() + 1);

    // if next elem is ok
    if (next_index.isValid()) {
        this->_playlistItemToUse = this->itemFromIndex(next_index);
        this->_requestPlay();
    } else {
        this->_playlistItemToUse = nullptr;
        this->_defaultPlay();
    }
}

void Playlist::playPrevious() {
    if (this->_defaultPlay()) return;  // if default play is used, do nothing

    // find previous elem
    auto selected_index = this->indexFromItem(this->_playlistItemToUse);
    auto prev_index = selected_index.siblingAtRow(selected_index.row() - 1);

    // if next elem is ok
    if (prev_index.isValid()) {
        this->_playlistItemToUse = this->itemFromIndex(prev_index);
        this->_requestPlay();
    } else {
        this->_playlistItemToUse = nullptr;
        this->_defaultPlay();
    }
}

bool Playlist::_defaultPlay() {
    if (!this->count()) return false;  // if no items, do nothing
    if (this->_playlistItemToUse) return false;  // if running link set, do nothing

    // select the first item, and play
    this->_playlistItemToUse = this->itemFromIndex(this->model()->index(0, 0));
    this->_requestPlay();
    return true;
}

void Playlist::_requestPlay() {
    auto currentPlay = this->currentPlay();
    if (currentPlay) emit playRequested(currentPlay);
}

void Playlist::_onItemDoubleClicked(QListWidgetItem * item) {
    this->_playlistItemToUse = item;
    this->_requestPlay();
}

AudioTube::VideoMetadata* Playlist::currentPlay() {
    if (!this->_playlistItemToUse) return nullptr;
    return RPZQVariant::ytVideoMetadata(this->_playlistItemToUse);
}

void Playlist::contextMenuEvent(QContextMenuEvent *event) {
    // display menu
    QMenu menu(this);
    menu.addAction(_getDeleteTrackAction(&menu));
    menu.exec(event->globalPos());
}

QAction* Playlist::_getDeleteTrackAction(QObject * parent) {
    auto action = new QAction(parent);
    auto selectedItems = this->selectedItems();
    action->setText(
        tr("Delete %1 track(s) from playlist").arg(selectedItems.count())
    );
    action->setIcon(QIcon(QStringLiteral(u":/icons/app/tools/bin.png")));
    QObject::connect(
        action, &QAction::triggered,
        this, &Playlist::_onCMTrackDeletionRequest
    );
    return action;
}

void Playlist::_onCMTrackDeletionRequest(bool checked) {
    Q_UNUSED(checked);
    this->_deleteSelectedTracks();
}

bool Playlist::_askTrackDeletion() {
    auto result = QMessageBox::warning(
        this,
        tr("Tracks deletion"),
        tr("Do you really want to delete theses %1 tracks ?").arg(this->selectedItems().count()),
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No
    );
    return result == QMessageBox::Yes ? true : false;
}