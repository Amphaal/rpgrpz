#pragma once

#include "src/_libs/qtPromise/qpromise.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace QtPromise;

class NetworkHelper {
    protected:
        static QPromise<QByteArray> download(const QUrl& url) {
            return QPromise<QByteArray>([&](
                const QPromiseResolve<QByteArray>& resolve,
                const QPromiseReject<QByteArray>& reject) {
                
                QNetworkRequest request(url);
                auto reply = _manager.get(request);

                QObject::connect(reply, &QNetworkReply::finished, [=]() {
                    if (reply->error() == QNetworkReply::NoError) {
                        resolve(reply->readAll());
                    } else {
                        reject(reply->error());
                    }

                    reply->deleteLater();
                });
            });
        }
    
    private:
        static inline QNetworkAccessManager _manager;
};