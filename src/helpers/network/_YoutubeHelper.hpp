#pragma once

#include <QList>
#include <QString>

#include <QRegularExpression>

#include <QJsonDocument>

#include "_NetworkHelper.hpp"

class YoutubeHelper : public NetworkHelper {
    public:
        static QPromise<QList<QString>> getVideoIdsFromPlaylistUrl(QString url) {
            
            //handler, search for list of videos
            auto _getYtInternalData = [](const QByteArray &data) {
                
                //to string for pcre2 usage
                auto str = QString::fromUtf8(data);

                //build regex
                QRegularExpression re("watch\\?v=(?<videoId>.*?)&amp;");

                //search...
                auto results = re.globalMatch(str);
                
                //fill the list
                QList<QString> idsList;
                while (results.hasNext()) {
                    QRegularExpressionMatch match = results.next();
                    if (match.hasMatch()) {
                        auto found = match.captured("videoId");
                        if(!idsList.contains(found)) {
                            idsList.append(found);
                        }
                    }
                }

                //if no ids
                if(!idsList.count()) throw new std::exception("no playlist metadata container found !");

                //return
                return idsList;
            };
            
            //chain
            return download(url).then(_getYtInternalData).fail([](QNetworkReply::NetworkError err) {
                qDebug() << err;
            });
                
        };

        static QString buildVideoUrlFromVideoId(QString videoId) {
            return QString("https://www.youtube.com/watch?v=") + videoId; 
        };

        static QPromise<QString> getVideoTitle(QString url) {
            
            //define request
            auto request = QString("https://www.youtube.com/oembed?url=") + url + "&format=json";

            //handler...
            auto _getTitleFromFetchedVideoInfo = [](const QByteArray &data) {                    
                //parse result into a JSONDoc
                QJsonParseError *error = nullptr;
                auto doc = QJsonDocument::fromJson(data, error);

                //resolve
                auto title = doc["title"].toString();
                return title;
            };

            //chain...
            return download(request).then(_getTitleFromFetchedVideoInfo);
        }
};