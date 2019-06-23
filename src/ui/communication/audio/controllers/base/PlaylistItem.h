#pragma once

#include <QString>
#include <QObject>

#include "src/helpers/network/youtube/YoutubeHelper.h"

#include "src/_libs/qtPromise/qpromise.h"

class PlaylistItem : public QObject {
    
    Q_OBJECT
    
    signals:
        void titleChanged(const QString &title);

    public:
        enum LinkType { YoutubePlaylist, YoutubeVideo, ServerAudio };
        
        PlaylistItem();
        PlaylistItem(LinkType type, const QString &uri);

        LinkType type();
        QString uri();
        QString title();
        QPromise<QString> streamSourceUri();
    
    private:
        YoutubeVideoMetadata* _mData = nullptr;
        
        LinkType _type;
        QString _title;
        QString _uri;

        void _setTitle(const QString &title);

        QPromise<void> _mayRefreshYTMetadata();
};