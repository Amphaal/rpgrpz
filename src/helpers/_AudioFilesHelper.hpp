#pragma once

#include <QString>

#include "libs/qtPromise/qpromise.h"

class AudioFilesHelper {
    public:
        static QPromise<QString> getTitleOfFile(QString uri) {
            return QPromise<QString>([uri](const auto& resolve, const auto& reject) {
                resolve(uri);
            });
        }
};