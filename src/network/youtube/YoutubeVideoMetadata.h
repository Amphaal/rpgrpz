#pragma once

#include <QDateTime>

#include <QHash>
#include <QString>
#include <QRegularExpression>

#include <QDebug>

#include "YoutubeAudioStreamInfos.h"

class YoutubeVideoMetadata {
    
    public:
        static YoutubeVideoMetadata* fromUrl(const QString &url);
        static QString urlFromVideoId(const QString &videoId);
        
        YoutubeVideoMetadata(const QString &videoId);

        QString id() const;
        QString title() const;
        QString url() const;
        QString sts() const;
        QString playerSourceUrl() const;
        int length();
        bool isValid();
        void setSts(const QString &sts);
        void setPlayerSourceUrl(const QString &pSourceUrl);
        void setTitle(const QString &title);
        void setLength(int lengthInSeconds);
        void setExpirationDate(const QDateTime &expiration);
        void setAudioStreamInfos(const YoutubeAudioStreamInfos &adaptativeStreamInfos);

        YoutubeAudioStreamInfos* audioStreams();

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