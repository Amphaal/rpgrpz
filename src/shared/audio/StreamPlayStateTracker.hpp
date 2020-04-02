// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include <QString>
#include <QDateTime>

#include <QVariantHash>

class StreamPlayStateTracker : public QVariantHash {
    
    public: 
        StreamPlayStateTracker() {};
        explicit StreamPlayStateTracker(const QVariantHash &hash) : QVariantHash(hash) {};

        void registerNewPlay(const QString &audioSourceUrl, const QString &sourceTitle, int durationInSecs) {
            this->_setPosInMsecs(0);
            this->_setDurationInSecs(durationInSecs);
            this->_setUrl(audioSourceUrl);
            this->_setTitle(sourceTitle);
            this->_setIsPlaying(true);
            this->_refreshUpdateTS();
        }

        void updatePlayingState(bool isPlaying) {
            
            //if going to pause, update position before updating TS
            if(!isPlaying && this->isPlaying()) {
                this->_setPosInMsecs(this->positionInMsecs()); 
            } 

            this->_setIsPlaying(isPlaying);
            this->_refreshUpdateTS();

        }
        
        void updatePositionInMSecs(qint64 posInMSecs) {
            this->_setPosInMsecs(posInMSecs);
            this->_refreshUpdateTS();
        }

        bool isSomethingPlaying() {
            return this->positionInMsecs() > -1;
        }

    ///
    ///
    ///

        qint64 positionInMsecs() const {
            
            auto lu = this->lastUpdate();
            auto pos = this->_posInMsecs();

            if(this->isPlaying() && !lu.isNull() && pos > -1) {

                auto now = QDateTime::currentDateTime();
                auto elapsed = lu.msecsTo(now);
                auto estimatedPos = pos + elapsed;

                if(estimatedPos >= this->durationInSecs() * 1000) return -1;
                else return (qint64)estimatedPos;

            }
                
            return pos;
        }

        int durationInSecs() const {
            return this->value(QStringLiteral(u"dur"), -1).toInt();
        }

        bool isPlaying() const {
            return this->value(QStringLiteral(u"plyn"), false).toBool();
        }

        const QString title() const {
            return this->value(QStringLiteral(u"titl")).toString();
        }

        const QString url() const {
            return this->value(QStringLiteral(u"url")).toString();
        }

        const QDateTime lastUpdate() const {
            return this->value(QStringLiteral(u"lu")).toDateTime();
        };

    private:
        qint64 _posInMsecs() const {
            return this->value(QStringLiteral(u"pos"), -1).value<qint64>();
        };

        void _setDurationInSecs(int newDurationInSecs) {
            this->insert(QStringLiteral(u"dur"), newDurationInSecs);
        };

        void _setIsPlaying(bool newIsPlaying) {
            this->insert(QStringLiteral(u"plyn"), newIsPlaying);
        };
        
        void _setTitle(const QString &newTitle) {
            this->insert(QStringLiteral(u"titl"), newTitle);
        }

        void _setUrl(const QString &newUrl) {
            this->insert(QStringLiteral(u"url"), newUrl);
        }

        void _refreshUpdateTS() {
            this->insert(QStringLiteral(u"lu"), QDateTime::currentDateTime());
        }
        
        void _setPosInMsecs(qint64 newPosInMsecs) {
            this->insert(QStringLiteral(u"pos"), newPosInMsecs);
        }

};