#pragma once

#include "src/_libs/promise.hpp"

#include <QNetworkReply>
#include <QNetworkAccessManager>

class NetworkHelper {
    protected:
        static promise::Defer download(const QUrl& url);

    private:
        static inline int _pending = 0;
};