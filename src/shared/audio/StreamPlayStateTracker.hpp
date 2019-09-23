#pragma once

#include <QString>
#include <QDateTime>

#include <QVariantHash>

class StreamPlayStateTracker : public QVariantHash {
    
    public: 
        StreamPlayStateTracker() {};
        StreamPlayStateTracker(const QVariantHash &hash) : QVariantHash(hash) {};

        void registerNewPlay(const QString &audioSourceUrl, const QString &sourceTitle, int durationInSecs) {
            this->_setPos(0);
            this->_setDuration(durationInSecs);
            this->_setUrl(audioSourceUrl);
            this->_setTitle(sourceTitle);
            this->_setIsPlaying(true);
            this->_refreshUpdateTS();
        }

        void updatePlayingState(bool isPlaying) {
            
            //if going to pause, update position before updating TS
            if(!isPlaying && this->isPlaying()) {
                this->_setPos(this->position()); 
            } 

            this->_setIsPlaying(isPlaying);
            this->_refreshUpdateTS();

        }
        
        void positionChanged(int pos) {
            this->_setPos(pos);
            this->_refreshUpdateTS();
        }

        bool isSomethingPlaying() {
            return this->position() > -1;
        }

    ///
    ///
    ///

        const int position() const {
            
            auto lu = this->lastUpdate();
            auto pos = this->_pos();

            if(this->isPlaying() && !lu.isNull() && pos > -1) {

                auto now = QDateTime::currentDateTime();
                auto elapsed = lu.secsTo(now);
                auto estimatedPos = pos + elapsed;

                if(estimatedPos >= this->duration()) return -1;
                else return (int)estimatedPos;

            }
                
            return pos;
        }

        const int duration() const {
            return this->value("dur", -1).toInt();
        }

        const bool isPlaying() const {
            return this->value("plyn", false).toBool();
        }

        const QString title() const {
            return this->value("titl").toString();
        }

        const QString url() const {
            return this->value("url").toString();
        }

        const QDateTime lastUpdate() const {
            return this->value("lu").toDateTime();
        };

    private:

        const int _pos() const {
            return this->value("pos", -1).toInt();
        };

        void _setDuration(int newDuration) {
            this->insert("dur", newDuration);
        };

        void _setIsPlaying(bool newIsPlaying) {
            this->insert("plyn", newIsPlaying);
        };
        
        void _setTitle(const QString &newTitle) {
            this->insert("titl", newTitle);
        }

        void _setUrl(const QString &newUrl) {
            this->insert("url", newUrl);
        }

        void _refreshUpdateTS() {
            this->insert("lu", QDateTime::currentDateTime());
        }
        
        void _setPos(int newPos) {
            this->insert("pos", newPos);
        }

};

Q_DECLARE_METATYPE(StreamPlayStateTracker)