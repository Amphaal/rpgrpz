#pragma once

#include <QString>
#include <QObject>

class PlaylistItem : public QObject {
    
    Q_OBJECT
    
    public:
        enum LinkType { YoutubePlaylist, YoutubeVideo, ServerAudio };
        
        PlaylistItem() {};
        PlaylistItem(const LinkType type, QString uri) : _type(type), _uri(uri) {};
        LinkType getType() {
            return this->_type;
        }

        QString getUri() {
            return this->_uri;
        }

        QString getTitle() {
            return this->_title;
        }

        void setTitle(QString title) {
            this->_title = title;
        }
    
    private:
        LinkType _type;
        QString _uri;
        QString _title;
};

Q_DECLARE_METATYPE(PlaylistItem*)