#pragma once

#include "src/_libs/qtPromise/qpromise.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace QtPromise;

class NetworkHelper {
    protected:
        static QPromise<QByteArray> download(const QUrl& url);
    
    private:
        static inline QNetworkAccessManager _manager;
};