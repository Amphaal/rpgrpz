#pragma once

#include "src/_libs/qtPromise/qpromise.h"

#include <QNetworkReply>
#include <QNetworkAccessManager>

using namespace QtPromise;

class NetworkHelper {
    protected:
        static QPromise<QByteArray> download(const QUrl& url);
};