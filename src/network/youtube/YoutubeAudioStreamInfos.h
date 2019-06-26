#pragma once

#include <QString>

#include <QHash>
#include <QString>
#include <QUrlQuery>

#include "YoutubeSignatureDecipherer.h"

#include <QDebug>

class YoutubeAudioStreamInfos {
    
    public:
        YoutubeAudioStreamInfos();
        YoutubeAudioStreamInfos(const QString &adaptativeStreamInfosAsStr, YoutubeSignatureDecipherer* decipherer);

        QString streamUrl(const QString &mime);
        QList<QString> availableAudioMimes();
        QPair<QString, QString> getPreferedMineSourcePair();
    
    private:
        QHash<QString, QHash<QString, QString>> _InfosByAudioMime;

        static QList<QHash<QString, QString>> _generatRawAdaptativeStreamInfos(const QString &adaptativeStreamInfosAsStr);
};