// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QByteArray>

#include "RPZAsset.hpp"

class RPZAssetImportPackage : public RPZAsset {
 public:
    RPZAssetImportPackage() {}
    explicit RPZAssetImportPackage(const QVariantHash &hash) : RPZAsset(hash) {}
    explicit RPZAssetImportPackage(const RPZAsset &asset) : RPZAsset(asset) {
        auto fp = this->filepath(false);
        QFile assetFile(fp);

        // check asset file existance
        if (!assetFile.exists()) return;

        // add to content
        this->insert(
            QStringLiteral(u"_content"),
            JSONSerializer::asBase64(assetFile)
        );

        this->_isSuccessful = true;
    }

    bool tryIntegratePackage() {
        auto bytes = this->assetAsBytes();
        if (!bytes.size() || this->isEmpty()) {
            qWarning() << "Assets : cannot integrate package as it does not contain a raw asset !";
            return false;
        }

        auto success = this->_integrateFrom(
            bytes,
            *this
        );

        if (success) this->_clearAssetAsBytes();

        return success;
    }

    bool isPackageCreationSuccessful() {
        return this->_isSuccessful;
    }

    const QByteArray assetAsBytes() const {
        return QByteArray::fromBase64(
            this->value(QStringLiteral(u"_content")).toByteArray()
        );
    }

 private:
    bool _isSuccessful = false;

    void _clearAssetAsBytes() {
        this->remove(QStringLiteral(u"_content"));
    }
};
