#pragma once

#include <QString>
#include <QDateTime>

class StreamPlayStateTracker {
    public:
        void registerNewPlay(const QString &audioSourceUrl, const QString &sourceTitle, int durationInSecs) {
            _pos = 0;
            _duration = durationInSecs;
            _url = audioSourceUrl;
            _title = sourceTitle;
            _isPlaying = true;

            this->_refreshUpdateTS();
        }
        void updateState(bool isPlaying) {
            
            //if going to pause, update position before updating TS
            if(!isPlaying && _isPlaying) {
                _pos = this->position(); 
            } 

            _isPlaying = isPlaying;
            this->_refreshUpdateTS();

        }
        
        void positionChanged(int pos) {
            this->_pos = pos;
            this->_refreshUpdateTS();
        }

        bool isSomethingPlaying() {
            return this->position() > -1;
        }

    ///
    ///
    ///

        const int position() const {
            
            if(_isPlaying && !_lastUpdate.isNull() && _pos) {

                auto now = QDateTime::currentDateTime();
                auto elapsed = _lastUpdate.secsTo(now);
                auto estimatedPos = _pos + elapsed;

                if(estimatedPos >= _duration) return -1;
                else return (int)estimatedPos;

            }
                
            return _pos;
        }

        const int duration() const {
            return _duration;
        }

        const bool isPlaying() const {
            return _isPlaying;
        }

        const QString title() const {
            return _title;
        }

        const QString url() const {
            return _url;
        }

    private:
        void _refreshUpdateTS() {
            this->_lastUpdate = QDateTime::currentDateTime();
        }

        int _pos = -1;
        int _duration = -1;
        bool _isPlaying = false;
        QString _url;
        QString _title;
        QDateTime _lastUpdate;
};