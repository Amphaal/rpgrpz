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

#include <audiotube/YoutubeHelper.h>

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
