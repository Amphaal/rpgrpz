#include "YoutubeAudioStreamInfos.h"

YoutubeAudioStreamInfos::YoutubeAudioStreamInfos() {};
YoutubeAudioStreamInfos::YoutubeAudioStreamInfos(const QString &adaptativeStreamInfosAsStr, YoutubeSignatureDecipherer* decipherer) {
    
    auto streamInfosByType = _generatRawAdaptativeStreamInfos(adaptativeStreamInfosAsStr);

    //filter, only audio
    for(auto &streamInfos : streamInfosByType) {
        auto type = streamInfos["type"];
        
        //exclude
        if(type.isEmpty()) continue;
        if(!type.contains("audio")) continue;

        //add to internal
        streamInfos.remove(type);
        this->_InfosByAudioMime.insert(type, streamInfos);
    }

    //alter url parameter if signature is needed
    for(auto &audioStreamInfos : this->_InfosByAudioMime) { 
        
        //signature
        auto baseSignature = audioStreamInfos["s"];
        if(baseSignature.isEmpty()) continue;

        //modify signature
        auto newSignature = decipherer->decipher(baseSignature);
        audioStreamInfos["s_deciphered"] = newSignature;

        //update url accordingly
        auto signatureParam = audioStreamInfos.value("sp", "signature");
        auto baseUrl = QUrl(audioStreamInfos["url"]);

            auto baseUrl_query = QUrlQuery(baseUrl.query());
            baseUrl_query.addQueryItem(signatureParam, newSignature);
            baseUrl.setQuery(baseUrl_query);

        audioStreamInfos["url"] = baseUrl.toString();
    }

}

QString YoutubeAudioStreamInfos::streamUrl(const QString &mime) {
    return this->_InfosByAudioMime[mime]["url"];
}

QList<QString> YoutubeAudioStreamInfos::availableAudioMimes() {
    return this->_InfosByAudioMime.keys();
}

QList<QHash<QString, QString>> YoutubeAudioStreamInfos::_generatRawAdaptativeStreamInfos(const QString &adaptativeStreamInfosAsStr) {

    auto out = QList<QHash<QString, QString>>();

    //for each group
    auto itagsDataGroupsAsStr = adaptativeStreamInfosAsStr.split(",", QString::SplitBehavior::SkipEmptyParts);
    for( auto &dataGroupAsString : itagsDataGroupsAsStr) {

        QHash<QString, QString> group;
        
        //for each pair
        auto pairs = dataGroupAsString.split("&", QString::SplitBehavior::SkipEmptyParts);
        for(auto &pair : pairs) {
            
            //current key/value pair
            auto kvpAsList = pair.split("=", QString::SplitBehavior::KeepEmptyParts);
            auto kvp = QPair<QString, QString>(kvpAsList[0], kvpAsList[1]);

            //add to temporary group
            group.insert(kvp.first, QUrlQuery(kvp.second).toString(QUrl::ComponentFormattingOption::FullyDecoded));

        }

        out.append(group);

    }

    return out;
}
