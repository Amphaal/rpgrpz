#pragma once

#include <QHash>
#include <QString>
#include <QRegularExpression>
#include <QPair>
#include <QQueue>
#include <QVariant>

#include <QDebug>

class YoutubeSignatureDecipherer {
    
    public:
        enum CipherOperation { CO_Unknown, Reverse, Slice, Swap };

        QString decipher(const QString &signature);
        static YoutubeSignatureDecipherer* create(const QString &clientPlayerUrl, const QString &rawPlayerSourceData);
        static YoutubeSignatureDecipherer* fromCache(const QString &clientPlayerUrl);

    private:
        YoutubeSignatureDecipherer(const QString &rawPlayerSourceData);
        
        QQueue<QPair<CipherOperation, QVariant>> _operations;
        static inline QHash<QString, YoutubeSignatureDecipherer*> _cache;
};