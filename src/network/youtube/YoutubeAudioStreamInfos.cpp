#include "YoutubeAudioStreamInfos.h"

YoutubeAudioStreamInfos::YoutubeAudioStreamInfos() {};
YoutubeAudioStreamInfos::YoutubeAudioStreamInfos(const QString &adaptativeStreamInfosAsStr, YoutubeSignatureDecipherer* decipherer) {
    
    auto streamInfosByType = _generatRawAdaptativeStreamInfos(adaptativeStreamInfosAsStr);

    //filter, only audio
    for(const auto &streamInfos : streamInfosByType) {
        auto type = streamInfos.value(QStringLiteral(u"type"));
        
        //exclude
        if(type.isEmpty()) continue;
        if(!type.contains(QStringLiteral(u"audio"))) continue;

        //add to internal
        streamInfos.remove(type);
        this->_InfosByAudioMime.insert(type, streamInfos);
    }

    //alter url parameter if signature is needed
    for(const auto &audioStreamInfos : this->_InfosByAudioMime) { 
        
        //signature
        auto baseSignature = audioStreamInfos.value(QStringLiteral(u"s"));
        if(baseSignature.isEmpty()) continue;

        //modify signature
        auto newSignature = decipherer->decipher(baseSignature);
        audioStreamInfos.insert(
            QStringLiteral(u"s_deciphered"), 
            newSignature
        );

        //update url accordingly
        auto signatureParam = audioStreamInfos.value(QStringLiteral(u"sp"), QStringLiteral(u"signature"));
        auto baseUrl = QUrl(audioStreamInfos.value(QStringLiteral(u"url")));

            auto baseUrl_query = QUrlQuery(baseUrl.query());
            baseUrl_query.addQueryItem(signatureParam, newSignature);
            baseUrl.setQuery(baseUrl_query);

        audioStreamInfos.insert(
            QStringLiteral(u"url"), 
            baseUrl.toString()
        );

    }

}

QPair<QString, QString> YoutubeAudioStreamInfos::getPreferedMineSourcePair() {
    auto available = this->availableAudioMimes();
    auto mp4Audio = available.filter(QRegularExpression(QStringLiteral(u"opus")));
    auto selectedMime = mp4Audio.count() ? mp4Audio.at(0) : available.at(0);
    auto selectedUrl = this->streamUrl(selectedMime);
    return QPair<QString, QString>(selectedMime, selectedUrl);
}

QString YoutubeAudioStreamInfos::streamUrl(const QString &mime) {
    return this->_InfosByAudioMime
                    .value(mime)
                    .value(QStringLiteral(u"url"));
}

QList<QString> YoutubeAudioStreamInfos::availableAudioMimes() {
    return this->_InfosByAudioMime.keys();
}

QList<QHash<QString, QString>> YoutubeAudioStreamInfos::_generatRawAdaptativeStreamInfos(const QString &adaptativeStreamInfosAsStr) {

    auto out = QList<QHash<QString, QString>>();

    //for each group
    auto itagsDataGroupsAsStr = adaptativeStreamInfosAsStr.split(
        QStringLiteral(u","), 
        QString::SplitBehavior::SkipEmptyParts
    );
    for( auto &dataGroupAsString : itagsDataGroupsAsStr) {

        QHash<QString, QString> group;
        
        //for each pair
        auto pairs = dataGroupAsString.split(
            QStringLiteral(u"&"), 
            QString::SplitBehavior::SkipEmptyParts
        );

        for(const auto &pair : pairs) {
            
            //current key/value pair
            auto kvpAsList = pair.split(
                QStringLiteral(u"="), 
                QString::SplitBehavior::KeepEmptyParts
            );
            auto kvp = QPair<QString, QString>(kvpAsList.at(0), kvpAsList.at(1));

            //add to temporary group
            group.insert(
                kvp.first, 
                QUrlQuery(kvp.second).toString(QUrl::ComponentFormattingOption::FullyDecoded)
            );

        }

        out.append(group);

    }

    return out;
}
