#pragma once

#include <QList>
#include <QString>

#include <QRegularExpression>
#include <QUrlQuery>

#include <QJsonDocument>

#include "_NetworkHelper.hpp"

class YoutubeVideo : public NetworkHelper {
    private:
        QString _videoId;
        QString _url;
        QString _title;
        int _lengthInSeconds = -1;

        QHash<int, QHash<QString, QString>>* _sourceUrlsByItag = nullptr;
        QHash<int, QString>* _audioTypeByItag = nullptr;

        void _setFormatsInfosFromQuery(QUrlQuery &query) {
            
            //delete / recreate
            if(this->_sourceUrlsByItag) delete this-> _sourceUrlsByItag;
            this->_sourceUrlsByItag = new QHash<int, QHash<QString, QString>>();
            
            //delete / recreate
            if(this->_audioTypeByItag) delete this-> _audioTypeByItag;
            this->_audioTypeByItag = new QHash<int, QString>();

            //find groups, separated by virgula (%2C)
            auto groups = query.queryItemValue("adaptive_fmts").split("%2C");
            auto innerDecode = QList<QString> {"type", "url"};
            
            for(auto &group : groups) {
                
                //temporary itag group
                QHash<QString, QString> t;
                auto isAudio = false;

                //iter...
                for(auto &pair : QUrlQuery(group).queryItems(QUrl::FullyDecoded)) {
                    
                    //redecode
                    auto data = pair.second;
                    if(innerDecode.contains(pair.first)) {
                        data = QUrl::fromPercentEncoding(data.toUtf8());
                    }
                    
                    //insert
                    t.insert(pair.first, data);
                    
                    qDebug() << pair.first << " : " << data;

                    //detect audio type
                    if(pair.first == "type" && data.contains("audio")) isAudio = true;
                }

                //insert it
                auto itag = t["itag"].toInt();
                this->_sourceUrlsByItag->insert(itag, t);

                //insert if audio type
                if(isAudio) this->_audioTypeByItag->insert(itag, t["type"]);

            }

        }


    public:
        YoutubeVideo(QString videoId) : _videoId(videoId), _url(urlFromVideoId(videoId)) {

        };

        QString getTitle() {
            return this->_title;
        }

        QString getUrl() {
            return this->_url;
        }

        QPromise<void> fetchVideoInfos() {
            
            //define request
            auto request = videoInfoRequestUrlFromVideoId(this->_videoId);

            //handler...
            auto _getInfos = [&](const QByteArray &data) {
                
                //as string then to query
                QUrlQuery query(QString::fromUtf8(data));

                //check for errors
                auto error = query.queryItemValue("errorcode");
                if(!error.isNull()) {
                    throw new std::exception("and error occured while fetching video infos");
                    return;
                }

                //format title
                auto title = query.queryItemValue("title", QUrl::FullyDecoded);
                title = title.replace("+", " ");
                
                //define title
                if(!title.isNull()) {
                    this->_title = title;
                }

                //set length
                this->_lengthInSeconds = query.queryItemValue("length_seconds").toInt();

                //define formats
                this->_setFormatsInfosFromQuery(query);

                return;
            };

            //chain...
            return download(request).then(_getInfos);
        }



        /////
        //// STATIC
        /////

        static QPromise<QList<QString>> fromPlaylistUrl(QString url) {
            
            //handler, search for list of videos
            auto _getYtInternalData = [](const QByteArray &data) {
                
                //to string for pcre2 usage
                auto str = QString::fromUtf8(data);

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
                if(!idsList.count()) throw new std::exception("no playlist metadata container found !");

                //return
                return idsList;
            };
        
            //chain
            return download(url).then(_getYtInternalData);         
        };

        static YoutubeVideo* fromUrl(QString url) {
            
            //find id
            QRegularExpression re("v=(?<videoId>.*?)($|&)");
            auto match = re.match(url);

            //returns
            if(!match.hasMatch()) {
                throw new std::exception("Cannot create YT video obj from url");
            }
            return new YoutubeVideo(match.captured("videoId"));
        }

        static QString urlFromVideoId(QString videoId) {
            return QString("https://www.youtube.com/watch?v=") + videoId;
        }

        static QString apiUrlFromVideoId(QString videoId) {
            return QString("https://youtube.googleapis.com/v/") +videoId;
        }

        static QString videoInfoRequestUrlFromVideoId(QString videoId) {
            auto encodedApiUrl = QUrl::toPercentEncoding(apiUrlFromVideoId(videoId));
            return QString("https://www.youtube.com/get_video_info?video_id=") + videoId + "&el=detailpage&eurl=" + encodedApiUrl;
        }

};