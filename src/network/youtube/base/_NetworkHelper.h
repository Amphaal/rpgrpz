#pragma once

#include "src/_libs/promise.hpp"

#include <QNetworkReply>
#include <QNetworkAccessManager>

class NetworkHelper {
    protected:
        static Defer download(const QUrl& url);

    private:
        static inline QNetworkAccessManager* _nam = nullptr;
        static QNetworkAccessManager* getNAM();
};