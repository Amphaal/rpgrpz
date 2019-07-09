#pragma once

#include <QListWidget>

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeDatabase>
#include <QMimeData>

#include "src/helpers/DurationHelper.hpp"

#include <QHash>
#include <QPair>
#include <QToolTip>

#include <QUrlQuery>

#include "src/network/youtube/YoutubeHelper.h"

class Playlist : public QListWidget {

    Q_OBJECT

    public:
        enum YoutubeUrlType { YoutubePlaylist, YoutubeVideo };
        Playlist(QWidget* parent = nullptr);

        void playNext();
        void playPrevious();

        void addYoutubeVideo(const QString &url);

        YoutubeVideoMetadata* currentPlay();
    
    signals:
        void playRequested(YoutubeVideoMetadata* metadata);

    private:

        QSet<QString> _playlistVideoIds;

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
};
