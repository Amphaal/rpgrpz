#include "_NetworkHelper.h"

QPromise<QByteArray> NetworkHelper::download(const QUrl& url) {
    return QPromise<QByteArray>([&](
        const QPromiseResolve<QByteArray>& resolve,
        const QPromiseReject<QByteArray>& reject) {
        
        QNetworkRequest request(url);
        auto reply = QNetworkAccessManager().get(request);

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