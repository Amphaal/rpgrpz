#pragma once

#include <QList>

#include <QDebug>

#include <QRegularExpression>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>

#include "src/helpers/network/base/_NetworkHelper.h"

#include "YoutubeVideoMetadata.h"

class YoutubeHelper : public NetworkHelper {
    
    public:
        static QPromise<QList<YoutubeVideoMetadata*>> fromPlaylistUrl(const QString &url);
        static QPromise<YoutubeVideoMetadata*> refreshMetadata(YoutubeVideoMetadata* toRefresh);

    private:
        static QPromise<QByteArray> _getVideoEmbedPageRawData(YoutubeVideoMetadata* metadata);
        static YoutubeVideoMetadata* _augmentMetadataWithPlayerConfiguration(YoutubeVideoMetadata* metadata, const QByteArray &videoEmbedPageRequestData);
        static YoutubeVideoMetadata* _augmentMetadataWithVideoInfos(YoutubeVideoMetadata* metadata, YoutubeSignatureDecipherer* decipherer, const QByteArray &videoInfoRawResponse, const QDateTime &tsRequest);

        static QPromise<YoutubeVideoMetadata*> _downloadVideoInfosAndAugmentMetadata(YoutubeVideoMetadata* metadata);
        static QString _getApiUrl(const QString &videoId);
        static QPromise<QByteArray> _getVideoInfosRawData(YoutubeVideoMetadata* metadata);
        static QList<QString> _extractVideoIdsFromHTTPRequest(const QByteArray &requestData);
        static QList<YoutubeVideoMetadata*> _videoIdsToMetadataList(const QList<QString> &videoIds);
};