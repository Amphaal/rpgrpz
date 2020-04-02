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

#pragma once

#include <QListWidget>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QMimeData>
#include <QTimer>

#include "src/helpers/StringHelper.hpp"

#include <QHash>
#include <QPair>
#include <QToolTip>

#include <QUrlQuery>

#include <audiotube/NetworkFetcher.h>

#include "src/helpers/RPZQVariant.hpp"

class Playlist : public QListWidget {

    Q_OBJECT

    public:
        enum class YoutubeUrlType { YoutubePlaylist, YoutubeVideo };
        Q_ENUM(YoutubeUrlType)

        Playlist(QWidget* parent = nullptr);

        void playNext();
        void playPrevious();

        void addYoutubeVideo(const QString &url);

        VideoMetadata* currentPlay();
    
    signals:
        void playRequested(VideoMetadata* metadata);

    private:

        QSet<VideoMetadata::Id> _playlistVideoIds;

        void _onItemDoubleClicked(QListWidgetItem * item);
        QListWidgetItem* _playlistItemToUse = nullptr;

        //drag and drop
            QMimeDatabase _MIMEDb;
            Qt::DropActions supportedDropActions() const override;
            void dragEnterEvent(QDragEnterEvent *event) override;
            void dragMoveEvent(QDragMoveEvent * event) override;
            void dropEvent(QDropEvent *event) override;

            //d&d temp
            QList<QPair<YoutubeUrlType, QUrl>> _tempDnD;
            int _tempHashDnDFromUrlList(QList<QUrl> &list);  

        bool _defaultPlay();
        void _requestPlay();

        //icons
        QIcon* _ytIconGrey = nullptr;
        QIcon* _ytIcon = nullptr;
        QIcon* _ytIconErr = nullptr;
};
