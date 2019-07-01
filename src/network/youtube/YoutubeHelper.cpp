#include "YoutubeHelper.h"

Defer YoutubeHelper::fromPlaylistUrl(const QString &url) {
    return download(url)
            .then(&_extractVideoIdsFromHTTPRequest)
            .then(&_videoIdsToMetadataList);         
};

Defer YoutubeHelper::refreshMetadata(YoutubeVideoMetadata* toRefresh) {
    return newPromise([=](Defer d){
        
        _getVideoEmbedPageRawData(toRefresh)
        .then([toRefresh](const QByteArray &htmlResponse) {
            return _augmentMetadataWithPlayerConfiguration(toRefresh, htmlResponse);
        })
        .then(&_downloadVideoInfosAndAugmentMetadata)
        .then([=]() {
            d.resolve(toRefresh);
        });

    });
};


Defer YoutubeHelper::_getVideoEmbedPageRawData(YoutubeVideoMetadata* metadata) {
    auto videoEmbedPageHtmlUrl = QString("https://www.youtube.com/embed/" + metadata->id() + "?disable_polymer=true&hl=en");
    return download(videoEmbedPageHtmlUrl);
}

YoutubeVideoMetadata* YoutubeHelper::_augmentMetadataWithPlayerConfiguration(YoutubeVideoMetadata* metadata, const QByteArray &videoEmbedPageRequestData) {
    //to string for pcre2 usage
    auto str = QString::fromUtf8(videoEmbedPageRequestData);

    //extract player config
    QRegularExpression re("yt\\.setConfig\\({'PLAYER_CONFIG': (?<playerConfig>.*?)}\\);");
    auto playerConfigAsStr = re.match(str).captured("playerConfig");
    auto playerConfig = QJsonDocument::fromJson(playerConfigAsStr.toUtf8()).object();
    auto args = playerConfig["args"].toObject();

    //get valuable data from it
    auto sts = playerConfig["sts"].toInt();
    auto playerSourceUrl = playerConfig["assets"].toObject()["js"].toString();
    auto title = args["title"].toString();
    auto length = args["length_seconds"].toInt();

    //check values exist
    if(!sts || playerSourceUrl.isEmpty() || title.isEmpty() || !length) {
        throw new std::logic_error("error while getting player client configuration !");
    }

    //augment...
    metadata->setSts(QString::number(sts));
    metadata->setPlayerSourceUrl("https://www.youtube.com" + playerSourceUrl);
    metadata->setTitle(title);
    metadata->setLength(length);

    return metadata;
}

YoutubeVideoMetadata* YoutubeHelper::_augmentMetadataWithVideoInfos(
    YoutubeVideoMetadata* metadata,
    YoutubeSignatureDecipherer* decipherer,
    const QByteArray &videoInfoRawResponse, 
    const QDateTime &tsRequest
) {
        
    //as string then to query, check for errors
    QUrlQuery videoInfos(QString::fromUtf8(videoInfoRawResponse));
    auto error = videoInfos.queryItemValue("errorcode");
    auto video_id = videoInfos.queryItemValue("video_id");
    if(!error.isNull() || video_id.isNull()) {
        throw new std::logic_error("An error occured while fetching video infos");
    }

    //define stream infos
    auto adaptativeStreamInfosAsStr = videoInfos.queryItemValue("adaptive_fmts", QUrl::ComponentFormattingOption::FullyDecoded);
    YoutubeAudioStreamInfos adaptativeStreamInfos(adaptativeStreamInfosAsStr, decipherer);
    metadata->setAudioStreamInfos(adaptativeStreamInfos);

    //player response as JSON, check if error
    auto playerResponseAsStr = videoInfos.queryItemValue("player_response", QUrl::ComponentFormattingOption::FullyDecoded);
    auto playerResponse = QJsonDocument::fromJson(playerResponseAsStr.toUtf8());
    auto pStatus = playerResponse["playabilityStatus"].toObject()["reason"].toString();
    if(!pStatus.isNull()) {
        throw new std::logic_error("An error occured while fetching video infos");
    }

    //set expiration flag
    auto expiresIn = playerResponse["streamingData"].toObject()["expiresInSeconds"].toString().toDouble();
    metadata->setExpirationDate(
        tsRequest.addSecs(expiresIn)
    );

    return metadata;
}


Defer YoutubeHelper::_downloadVideoInfosAndAugmentMetadata(YoutubeVideoMetadata* metadata) {
    
    //define timestamp for request
    auto requestedAt = QDateTime::currentDateTime();
    auto cachedDecipherer = YoutubeSignatureDecipherer::fromCache(metadata->playerSourceUrl());

    //helper for raw data download
    QVector<Defer> dlPromises{
        _getVideoInfosRawData(metadata),
        !cachedDecipherer ? download(metadata->playerSourceUrl()) : promise::resolve(QByteArray())
    };
    auto downloadAll = Defer::all(dlPromises);

    //handle augment
    return downloadAll.then([&, metadata, requestedAt, cachedDecipherer](const QVector<QByteArray>& res) {
        
        //prepare args
        auto videoInfosRawData = res[0];
        auto c_decipherer = cachedDecipherer ? cachedDecipherer : YoutubeSignatureDecipherer::create(
                                                                    metadata->playerSourceUrl(),
                                                                    QString::fromUtf8(res[1])
                                                                    );

        //augment
        return _augmentMetadataWithVideoInfos(metadata, c_decipherer, videoInfosRawData, requestedAt);

    });
}

QString YoutubeHelper::_getApiUrl(const QString &videoId) {
    return QString("https://youtube.googleapis.com/v/") +videoId;
}

Defer YoutubeHelper::_getVideoInfosRawData(YoutubeVideoMetadata* metadata) {
    auto encodedApiUrl = QUrl::toPercentEncoding(_getApiUrl(metadata->id()));
    auto requestUrl = QString("https://www.youtube.com/get_video_info?video_id=" + metadata->id() + "&el=embedded&sts=" + metadata->sts() + "&eurl=" + encodedApiUrl + "&hl=en");
    return download(requestUrl);
}


QList<QString> YoutubeHelper::_extractVideoIdsFromHTTPRequest(const QByteArray &requestData) {
        
        //to string for pcre2 usage
        auto str = QString::fromUtf8(requestData);

        //build regex
        QRegularExpression re("watch\\?v=(?<videoId>.*?)&amp;");

        //search...
        auto results = re.globalMatch(str);
        
        //return list
        QList<QString> idsList;

        //iterate
        while (results.hasNext()) {
            QRegularExpressionMatch match = results.next(); //next
            
            //if match
            if (match.hasMatch()) {
                auto found = match.captured("videoId"); //videoId
                
                if(!idsList.contains(found)) {
                    idsList.append(found);
                }
            }
        }

        //if no ids
        if(!idsList.count()) throw new std::logic_error("no playlist metadata container found !");

        //return
        return idsList;
}

QList<YoutubeVideoMetadata*> YoutubeHelper::_videoIdsToMetadataList(const QList<QString> &videoIds) {
    QList<YoutubeVideoMetadata*> out;
    for(auto &id : videoIds) {
        out.append(new YoutubeVideoMetadata(id));
    } 
    return out;
};
