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
        default:
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

int PlaylistItem::durationSecs() {
    return this->_duration;
}

promise::Defer PlaylistItem::streamSourceUri() {
    switch(this->_type) {
        case PlaylistItem::LinkType::ServerAudio: {
                return promise::resolve(this->_uri);
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
        
        default:
            break;
    }

    return promise::resolve("");
}

promise::Defer PlaylistItem::_mayRefreshYTMetadata() {
    if(this->_type == PlaylistItem::LinkType::ServerAudio) return promise::resolve();
    if(!this->_mData) return promise::resolve();
    if(this->_mData->isValid()) return promise::resolve();

    return YoutubeHelper::refreshMetadata(this->_mData).then([=]() {
        
        this->_title = this->_mData->title();
        this->_duration = this->_mData->duration();
        
        emit metadataChanged(this->_mData);

    });
}