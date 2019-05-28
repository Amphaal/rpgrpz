#pragma once

#include <QDateTime>

#include <QHash>
#include <QString>
#include <QRegularExpression>

#include <QDebug>

#include "YoutubeAudioStreamInfos.hpp"

class YoutubeVideoMetadata {
    
    public:
        static YoutubeVideoMetadata* fromUrl(const QString &url) {
            
            //find id
            QRegularExpression re("v=(?<videoId>.*?)($|&)");
            auto match = re.match(url);

            //returns
            if(!match.hasMatch()) {
                throw new std::exception("Cannot create YT video obj from url");
            }

            return new YoutubeVideoMetadata(match.captured("videoId"));
        }

        static QString urlFromVideoId(const QString &videoId) {
            return QString("https://www.youtube.com/watch?v=") + videoId;
        }
        
        YoutubeVideoMetadata(const QString &videoId) : _videoId(videoId), _url(urlFromVideoId(videoId)) { };

        QString id() const {
            return this->_videoId;
        }

        QString title() const {
            return this->_title;
        }

        QString url() const {
            return this->_url;
        }

        QString sts() const {
            return this->_sts;
        }

        QString playerSourceUrl() const {
            return this->_playerSourceUrl;
        }

        int length() {
            return this->_lengthInSeconds;
        }

        bool isValid() {
            if(this->_validUntil.isNull()) return false;
            return this->_validUntil < QDateTime::currentDateTime();
        }

        void setSts(const QString &sts) {
            this->_sts = sts;
        };

        void setPlayerSourceUrl(const QString &pSourceUrl) {
            this->_playerSourceUrl = pSourceUrl;
        }

        void setTitle(const QString &title) {
            this->_title = title;
        }

        void setLength(int lengthInSeconds) {
            this->_lengthInSeconds = lengthInSeconds;
        }

        void setExpirationDate(const QDateTime &expiration) {
            this->_validUntil = expiration;
        }

        void setAudioStreamInfos(const YoutubeAudioStreamInfos &adaptativeStreamInfos) {
            this->_audioStreamInfos = adaptativeStreamInfos;
        }

        YoutubeAudioStreamInfos* audioStreams() {
            return &this->_audioStreamInfos;
        }

    private:
        int _lengthInSeconds = -1;
        QString _videoId;
        QString _url;
        QString _title;

        QString _sts;
        QString _playerSourceUrl;

        YoutubeAudioStreamInfos _audioStreamInfos;

        QDateTime _validUntil;

        QHash<int, QHash<QString, QString>> _sourceUrlsByItag;
        QHash<int, QString> _audioTypeByItag;
};