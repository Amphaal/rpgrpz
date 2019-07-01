#pragma once

#include <QList>

#include <QDebug>

#include <QRegularExpression>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>

#include "base/_NetworkHelper.h"

#include "YoutubeVideoMetadata.h"

class YoutubeHelper : public NetworkHelper {
    
    public:
        static Defer fromPlaylistUrl(const QString &url);
        static Defer refreshMetadata(YoutubeVideoMetadata* toRefresh);

    private:
        static Defer _getVideoEmbedPageRawData(YoutubeVideoMetadata* metadata);
        static YoutubeVideoMetadata* _augmentMetadataWithPlayerConfiguration(YoutubeVideoMetadata* metadata, const QByteArray &videoEmbedPageRequestData);
        static YoutubeVideoMetadata* _augmentMetadataWithVideoInfos(YoutubeVideoMetadata* metadata, YoutubeSignatureDecipherer* decipherer, const QByteArray &videoInfoRawResponse, const QDateTime &tsRequest);

        static Defer _downloadVideoInfosAndAugmentMetadata(YoutubeVideoMetadata* metadata);
        static QString _getApiUrl(const QString &videoId);
        static Defer _getVideoInfosRawData(YoutubeVideoMetadata* metadata);
        static QList<QString> _extractVideoIdsFromHTTPRequest(const QByteArray &requestData);
        static QList<YoutubeVideoMetadata*> _videoIdsToMetadataList(const QList<QString> &videoIds);
};