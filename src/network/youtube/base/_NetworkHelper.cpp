#include "_NetworkHelper.h"

QNetworkAccessManager* NetworkHelper::_getNAM() {
    if(!_nam) _nam = new QNetworkAccessManager;
    return _nam;
}

promise::Defer NetworkHelper::download(const QUrl& url) {

    return promise::newPromise([=](promise::Defer d) {

        QNetworkRequest request(url);
        auto manager = _getNAM();
        auto reply = manager->get(request);  

        //on finished
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                auto result = reply->readAll();
                d.resolve(result);
            } else {
                d.reject(reply->error());       
            }

            delete reply;
        });
    });
}