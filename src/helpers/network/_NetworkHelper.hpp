#pragma once

#include "libs/qtPromise/qpromise.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace QtPromise;

class NetworkHelper {
    protected:
        static QPromise<QByteArray> download(const QUrl& url) {
            return QPromise<QByteArray>([&](
                const QPromiseResolve<QByteArray>& resolve,
                const QPromiseReject<QByteArray>& reject) {

                QNetworkReply* reply = _manager.get(QNetworkRequest(url));
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
        static inline QNetworkAccessManager _manager = QNetworkAccessManager();
};