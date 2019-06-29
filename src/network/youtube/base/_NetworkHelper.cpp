#include "_NetworkHelper.h"

QPromise<QByteArray> NetworkHelper::download(const QUrl& url) {
    return QPromise<QByteArray>([&](
        const QPromiseResolve<QByteArray>& resolve,
        const QPromiseReject<QByteArray>& reject) {
        
        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        auto reply = manager.get(request);  

        //on standard error
        QObject::connect(
            reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), [=](QNetworkReply::NetworkError code) {
                reject(code);
                reply->deleteLater();
            }
        );

        //on ssl error 
        QObject::connect(reply, &QNetworkReply::sslErrors, [=](const QList<QSslError> &errors) {
            qDebug() << errors;
            reject("SSL Error");
            reply->deleteLater();
        });

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