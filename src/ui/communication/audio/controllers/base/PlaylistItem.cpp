#include "PlaylistItem.h"
        
PlaylistItem::PlaylistItem() {};
PlaylistItem::PlaylistItem(LinkType type, const QString &uri) : _type(type), _uri(uri) {
    
    //conditionnal additionnal informations fetchers
    switch(type) {
        case PlaylistItem::LinkType::YoutubePlaylist:
        case PlaylistItem::LinkType::YoutubeVideo: {
                this->_mData = YoutubeVideoMetadata::fromUrl(uri);
            }
            break;
    }
};
PlaylistItem::LinkType PlaylistItem::type() {
    return this->_type;
}

QString PlaylistItem::uri() {
    return this->_uri;
}

QString PlaylistItem::title() {
    return this->_title;
}

QPromise<QString> PlaylistItem::streamSourceUri() {
    switch(this->_type) {
        case PlaylistItem::LinkType::ServerAudio: {
                return QPromise<QString>::resolve(this->_uri);
            }
            break;
        case PlaylistItem::LinkType::YoutubePlaylist:
        case PlaylistItem::LinkType::YoutubeVideo: {
                return this->_mayRefreshYTMetadata().then([=]() {
                    auto pair = this->_mData->audioStreams()->getPreferedMineSourcePair();
                    return pair.second;
                });
            }
            break;
    }

    return QPromise<QString>::resolve("");
}


void PlaylistItem::_setTitle(const QString &title) {
    this->_title = title;
    emit titleChanged(title);
}

QPromise<void> PlaylistItem::_mayRefreshYTMetadata() {
    if(this->_type == PlaylistItem::LinkType::ServerAudio) return QPromise<void>::resolve();
    if(!this->_mData) return QPromise<void>::resolve();
    if(this->_mData->isValid()) return QPromise<void>::resolve();

    return YoutubeHelper::refreshMetadata(this->_mData).then([=]() {
        this->_setTitle(this->_mData->title());
    });
}