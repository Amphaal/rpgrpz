#pragma once

#include <QString>
#include <QObject>

#include "src/helpers/network/youtube/YoutubeHelper.hpp"
#include "src/helpers/_AudioFilesHelper.hpp"

#include "libs/qtPromise/qpromise.h"

class PlaylistItem : public QObject {
    
    Q_OBJECT
    
    signals:
        void titleChanged(const QString &title);

    public:
        enum LinkType { YoutubePlaylist, YoutubeVideo, ServerAudio };
        
        PlaylistItem() {};
        PlaylistItem(LinkType type, const QString &uri) : _type(type), _uri(uri) {
            
            //conditionnal additionnal informations fetchers
            switch(type) {
                case PlaylistItem::LinkType::YoutubePlaylist:
                case PlaylistItem::LinkType::YoutubeVideo: {
                        this->_mData = YoutubeVideoMetadata::fromUrl(uri);
                    }
                    break;
            }
        };
        LinkType type() {
            return this->_type;
        }

        QString uri() {
            return this->_uri;
        }

        QString title() {
            return this->_title;
        }

        QPromise<QString> streamSourceUri() {
            switch(this->_type) {
                case PlaylistItem::LinkType::ServerAudio: {
                        return AudioFilesHelper::getTitleOfFile(this->_uri).then([=](const QString &title) {
                            this->_setTitle(title);
                            return this->_uri;
                        });
                    }
                    break;
                case PlaylistItem::LinkType::YoutubePlaylist:
                case PlaylistItem::LinkType::YoutubeVideo: {
                        return this->_mayRefreshYTMetadata().then([=]() {
                            return this->_getPreferedStreamSourceFromYTMetadata();
                        });
                    }
                    break;
            }

            return QPromise<QString>::resolve("");
        }

    
    private:
        YoutubeVideoMetadata* _mData = nullptr;
        
        LinkType _type;
        QString _title;
        QString _uri;

        void _setTitle(const QString &title) {
            this->_title = title;
            emit titleChanged(title);
        }

        QPromise<void> _mayRefreshYTMetadata() {
            if(this->_type == PlaylistItem::LinkType::ServerAudio) return QPromise<void>::resolve();
            if(!this->_mData) return QPromise<void>::resolve();
            if(this->_mData->isValid()) return QPromise<void>::resolve();

            return YoutubeHelper::refreshMetadata(this->_mData).then([=]() {
                this->_setTitle(this->_mData->title());
            });
        }
        
        QString _getPreferedStreamSourceFromYTMetadata() {
            auto available = this->_mData->audioStreams()->availableAudioMimes();
            auto selectedMime = available.takeFirst();
            return this->_mData->audioStreams()->streamUrl(selectedMime);
        }
};