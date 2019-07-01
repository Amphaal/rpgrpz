#include "_NetworkHelper.h"

QNetworkAccessManager* NetworkHelper::getNAM() {
    if(!_nam) _nam = new QNetworkAccessManager;
    return _nam;
}

Defer NetworkHelper::download(const QUrl& url) {

    return newPromise([=](Defer d) {

        QNetworkRequest request(url);
        QNetworkAccessManager manager;
        auto reply = getNAM()->get(request);  

        //on finished
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                d.resolve(reply->readAll());
            } else {
                d.reject(reply->error());
            }

            reply->deleteLater();
        });
    });
}