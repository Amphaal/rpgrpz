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

#include <QVariantHash>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

class RPZSharedDocument : public QVariantHash {
 public:
    using FileHash = QString;
    using DocumentName = QString;
    using NamesStore = QHash<RPZSharedDocument::FileHash, RPZSharedDocument::DocumentName>;
    using Store = QHash<RPZSharedDocument::FileHash, RPZSharedDocument>;

    RPZSharedDocument() {}
    explicit RPZSharedDocument(const QVariantHash &hash) : QVariantHash(hash) {}
    explicit RPZSharedDocument(const QUrl &localFileUrl) {
        this->_inst(localFileUrl);
    }

    friend QDebug operator<<(QDebug debug, const RPZSharedDocument &c) {
        QDebugStateSaver saver(debug);

        auto frtd_bs = QLocale::system().formattedDataSize(c.documentBytesSize());
        debug.nospace() << QStringLiteral("%1.%2 [%3 - %4]")
            .arg(c.documentName())
            .arg(c.documentExt())
            .arg(c.documentFileHash())
            .arg(frtd_bs);

        return debug;
    }

    static RPZSharedDocument::NamesStore toNamesStore(const QVariantHash &hash) {
        RPZSharedDocument::NamesStore out;

        for (auto i = hash.begin(); i != hash.end(); i++) {
            auto &hash = i.key();
            auto name = i.value().toString();
            out.insert(hash, name);
        }

        return out;
    }

    static QVariantHash toVariantNamesStore(const RPZSharedDocument::NamesStore &namesStore) {
        QVariantHash out;

        for (auto i = namesStore.begin(); i != namesStore.end(); i++) {
            auto &hash = i.key();
            auto name = i.value();
            out.insert(hash, name);
        }

        return out;
    }

    QString documentName() const {
        return this->value(QStringLiteral(u"nm")).toString();
    }

    QByteArray document() const {
        auto base64 = this->value(QStringLiteral(u"doc")).toByteArray();
        return JSONSerializer::toBytes(base64);
    }

    double documentBytesSize() const {
        return this->value(QStringLiteral("docS")).toDouble();
    }

    RPZSharedDocument::FileHash documentFileHash() const {
        return this->value(QStringLiteral(u"fileH")).toString();
    }

    bool isSuccess() const {
        return this->_localInstSuccess;
    }

    QString documentExt() const {
        return this->value(QStringLiteral(u"ext")).toString();
    }

    QString writeAsTemporaryFile() const {
        auto filePath = AppContext::getFileSharingFolderLocation() + "/" + this->documentFileHash() + "." + this->documentExt();

        QFile fHandler(filePath);
        auto opened = fHandler.open(QIODevice::WriteOnly);
        if (opened) {
            fHandler.write(this->document());
            fHandler.close();
        }

        return filePath;
    }

 private:
    bool _localInstSuccess = false;

    void _inst(const QUrl &localFileUrl) {
        // check if url is local
        if (!localFileUrl.isLocalFile()) {
            qDebug() << qUtf8Printable(QStringLiteral("File Share : %1 is not a local file !").arg(localFileUrl.toString()));
            return;
        }

        // check if file exists
        auto fullPath = localFileUrl.toLocalFile();
        QFileInfo fi(fullPath);

        if (!fi.exists()) {
            qDebug() << qUtf8Printable(QStringLiteral("File Share : %1 does not exist anymore !").arg(localFileUrl.toString()));
            return;
        }

        // read file
        QFile reader(fullPath);
        auto bytes = JSONSerializer::asBase64(reader);

        // override file if exists
        auto hash = RPZSharedDocument::_getFileHash(bytes);
        auto ext = fi.suffix();
        auto name = fi.completeBaseName();

        // insert in obj
        this->insert(QStringLiteral(u"fileH"), hash);
        this->insert(QStringLiteral(u"nm"), name);
        this->insert(QStringLiteral(u"doc"), bytes);
        this->insert(QStringLiteral(u"docS"), bytes.count());
        this->insert(QStringLiteral(u"ext"), ext);

        //
        this->_localInstSuccess = true;
    }

    static RPZSharedDocument::FileHash _getFileHash(const QByteArray &fileBytes) {
        // read signature...
        auto hash = QString::fromUtf8(
            QCryptographicHash::hash(
                fileBytes,
                QCryptographicHash::Keccak_224
            ).toHex()
        );

        return hash;
    }
};
Q_DECLARE_METATYPE(RPZSharedDocument)
