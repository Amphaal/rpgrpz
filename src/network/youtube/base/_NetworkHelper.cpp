#include "_NetworkHelper.h"

QNetworkAccessManager* NetworkHelper::getNAM() {
    if(!_nam) _nam = new QNetworkAccessManager;
    return _nam;
}

QPromise<QByteArray> NetworkHelper::download(const QUrl& url) {

    return QPromise<QByteArray>([=](
        const QPromiseResolve<QByteArray>& resolve,
        const QPromiseReject<QByteArray>& reject) {

        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        auto reply = getNAM()->get(request);  

        //on finished
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