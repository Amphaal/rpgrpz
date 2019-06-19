#include "YoutubeVideoMetadata.h"

YoutubeVideoMetadata* YoutubeVideoMetadata::fromUrl(const QString &url) {
    
    //find id
    QRegularExpression re("v=(?<videoId>.*?)($|&)");
    auto match = re.match(url);

    //returns
    if(!match.hasMatch()) {
        throw new std::exception("Cannot create YT video obj from url");
    }

    return new YoutubeVideoMetadata(match.captured("videoId"));
}

QString YoutubeVideoMetadata::urlFromVideoId(const QString &videoId) {
    return QString("https://www.youtube.com/watch?v=") + videoId;
}

YoutubeVideoMetadata::YoutubeVideoMetadata(const QString &videoId) : _videoId(videoId), _url(urlFromVideoId(videoId)) { };

QString YoutubeVideoMetadata::id() const {
    return this->_videoId;
}

QString YoutubeVideoMetadata::title() const {
    return this->_title;
}

QString YoutubeVideoMetadata::url() const {
    return this->_url;
}

QString YoutubeVideoMetadata::sts() const {
    return this->_sts;
}

QString YoutubeVideoMetadata::playerSourceUrl() const {
    return this->_playerSourceUrl;
}

int YoutubeVideoMetadata::length() {
    return this->_lengthInSeconds;
}

bool YoutubeVideoMetadata::isValid() {
    if(this->_validUntil.isNull()) return false;
    return this->_validUntil < QDateTime::currentDateTime();
}

void YoutubeVideoMetadata::setSts(const QString &sts) {
    this->_sts = sts;
};

void YoutubeVideoMetadata::setPlayerSourceUrl(const QString &pSourceUrl) {
    this->_playerSourceUrl = pSourceUrl;
}

void YoutubeVideoMetadata::setTitle(const QString &title) {
    this->_title = title;
}

void YoutubeVideoMetadata::setLength(int lengthInSeconds) {
    this->_lengthInSeconds = lengthInSeconds;
}

void YoutubeVideoMetadata::setExpirationDate(const QDateTime &expiration) {
    this->_validUntil = expiration;
}

void YoutubeVideoMetadata::setAudioStreamInfos(const YoutubeAudioStreamInfos &adaptativeStreamInfos) {
    this->_audioStreamInfos = adaptativeStreamInfos;
}

YoutubeAudioStreamInfos* YoutubeVideoMetadata::audioStreams() {
    return &this->_audioStreamInfos;
}

