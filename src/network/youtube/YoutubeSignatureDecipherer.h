////////////////////////////////////////////////////////////////////////////
// BASED ON THE WORK OF Alexey "Tyrrrz" Golub (https://github.com/Tyrrrz) //
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QHash>
#include <QString>
#include <QRegularExpression>
#include <QPair>
#include <QQueue>
#include <QVariant>

#include <QDebug>

enum YTCipherOperation { CO_Unknown, Reverse, Slice, Swap };
typedef QQueue<QPair<YTCipherOperation, QVariant>> YTDecipheringOperations;
typedef QString YTClientMethod;

class YoutubeSignatureDecipherer {
    
    public:
        QString decipher(const QString &signature);
        static YoutubeSignatureDecipherer* create(const QString &clientPlayerUrl, const QString &rawPlayerSourceData);
        static YoutubeSignatureDecipherer* fromCache(const QString &clientPlayerUrl);

    private:
        YoutubeSignatureDecipherer(const QString &rawPlayerSourceData);
        
        QQueue<QPair<YTCipherOperation, QVariant>> _operations;
        static inline QHash<QString, YoutubeSignatureDecipherer*> _cache;

        static YTClientMethod _findObfuscatedDecipheringFunctionName(const QString &ytPlayerSourceCode);
        static QList<QString> _findJSDecipheringOperations(const QString &ytPlayerSourceCode, const YTClientMethod &obfuscatedDecipheringFunctionName);
        static QHash<YTCipherOperation, YTClientMethod> _findObfuscatedDecipheringOperationsFunctionName(const QString &ytPlayerSourceCode, QList<QString> &javascriptDecipheringOperations);
        static YTDecipheringOperations _buildOperations(QHash<YTCipherOperation, YTClientMethod> &functionNamesByOperation, QList<QString> &javascriptOperations);
};
