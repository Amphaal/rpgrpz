#pragma once

#include <QList>

#include <QDebug>

#include <QRegularExpression>
#include <QUrlQuery>
#include <QString>

#include <QJsonDocument>
#include <QJsonObject>

#include "base/_NetworkHelper.h"

#include "YoutubeVideoMetadata.h"

class YoutubeHelper : public NetworkHelper {
    
    public:
        static promise::Defer fromPlaylistUrl(const QString &url);
        static promise::Defer refreshMetadata(YoutubeVideoMetadata* toRefresh);

    private:
        static promise::Defer _getVideoEmbedPageRawData(YoutubeVideoMetadata* metadata);
        static YoutubeVideoMetadata* _augmentMetadataWithPlayerConfiguration(YoutubeVideoMetadata* metadata, const QByteArray &videoEmbedPageRequestData);
        static YoutubeVideoMetadata* _augmentMetadataWithVideoInfos(YoutubeVideoMetadata* metadata, YoutubeSignatureDecipherer* decipherer, const QByteArray &videoInfoRawResponse, const QDateTime &tsRequest);

        static promise::Defer _downloadVideoInfosAndAugmentMetadata(YoutubeVideoMetadata* metadata);
        static QString _getApiUrl(const QString &videoId);
        static promise::Defer _getVideoInfosRawData(YoutubeVideoMetadata* metadata);
        static QList<QString> _extractVideoIdsFromHTTPRequest(const QByteArray &requestData);
        static QList<YoutubeVideoMetadata*> _videoIdsToMetadataList(const QList<QString> &videoIds);
};