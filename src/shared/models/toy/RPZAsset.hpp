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

#include <QSize>
#include <QPointF>
#include <QVariantHash>
#include <QUrl>
#include <QFile>
#include <QCryptographicHash>
#include <QImageReader>
#include <QFileInfo>
#include <QPixmapCache>

#include "src/helpers/_appContext.h"
#include "src/helpers/JSONSerializer.h"

class RPZAsset : public QVariantHash {
    public:
        using Hash = QString; //file hash of the asset

        RPZAsset() {}
        explicit RPZAsset(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZAsset(const QVariantHash &hash, const RPZAsset::Hash &id) : QVariantHash(hash) {
            this->_setHash(id);
        }
        RPZAsset(const QUrl &uri) {
            this->_integrateFrom(uri);
        }

        static const QPixmap cachedIconPixmap(const RPZAsset &asset, QSize &sizeToApply) {
            
            QPixmap cached;

            //search in cache
            auto idToSearch = asset.hash() + QStringLiteral(u"_ico");
            auto isFound = QPixmapCache::find(idToSearch, &cached);
            if(isFound) return cached;

            //get asset from filepath
            QPixmap pixmap(asset.filepath());

            //resize it to hint
            pixmap = pixmap.scaled(
                sizeToApply,
                Qt::AspectRatioMode::KeepAspectRatio 
            );

            //cache pixmap and return it
            QPixmapCache::insert(idToSearch, pixmap);
            return pixmap;

        }

        const QRectF shapeRect() const { 
            auto size = JSONSerializer::toQSize(this->value(QStringLiteral(u"shape")).toList()); 
            return QRectF(QPointF(), size);
        }
        const QPointF shapeCenter() const {
            return JSONSerializer::toPointF(this->value(QStringLiteral(u"center")).toList());
        }
        const QString fileExtension() const {
            return this->value(QStringLiteral(u"ext")).toString();
        }
        const QString name() const {
            return this->value(QStringLiteral(u"name")).toString();
        }
        const RPZAsset::Hash hash() const { 
            return this->value(QStringLiteral(u"hash")).toString(); 
        }

        bool isValidAsset() const {
            return !this->hash().isEmpty();
        }

        const QString filepath(bool checkExistance = true) const {
            
            auto expected = _getFilePathToAsset(
                this->hash(),
                this->fileExtension()
            );
            
            if(!checkExistance) return expected;

            auto exists = QFileInfo::exists(expected);
            if(!exists) {
                qWarning() << qUtf8Printable(QString("Assets : non-existent %1 asset file being invoked").arg(expected));
            }

            return exists ? expected : QString();

        }

        void updateAssetGeometryData() {
            
            auto ext = this->fileExtension();
            auto filePath = this->filepath();
            
            QFile fileReader(filePath);
            if(!fileReader.exists()) {
                qDebug() << "Assets : cannot update geometry, asset file does not exist !";
                return;
            }

            this->_updateAssetGeometryData(fileReader, ext);

        }

        void rename(const QString &newName) {
            this->insert(QStringLiteral(u"name"), newName);
        }

    protected:
        bool _integrateFrom(const QByteArray &assetAsRawBytes, const RPZAsset &asset) const {

            auto dest = asset.filepath(false);
            QFile fileWriter(dest);

            //check existance
            if(fileWriter.exists()) return false;

            //write asset            
            fileWriter.open(QIODevice::WriteOnly);
                fileWriter.write(assetAsRawBytes);
            fileWriter.close();

            return true;

        }

    private:
        void _setHash(const RPZAsset::Hash &hash) {
            this->insert(QStringLiteral(u"hash"), hash);
        }

        void _updateAssetGeometryData(QFile &fileReader, const QString &fileExtension) {
            
            //image metadata
            QImageReader imgReader(&fileReader, qUtf8Printable(fileExtension));
            auto assetSize = imgReader.size();
            if(!assetSize.isValid()) {
                qDebug() << "Assets : cannot update asset geometry as the geometry is unoptainable !";
                return;
            }

            auto assetCenter = QPointF(
                (qreal)assetSize.width() / 2,
                (qreal)assetSize.height() / 2
            );

            this->insert(QStringLiteral(u"shape"), JSONSerializer::fromQSize(assetSize));
            this->insert(QStringLiteral(u"center"), JSONSerializer::fromPointF(assetCenter));

        }

        void _integrateFrom(const QUrl &uri) {
            
            if(!uri.isLocalFile()) {
                qDebug() << "Assets : cannot create asset, source URI is not a file !";
                return;
            }

            //check file exists
            QFile fileReader(uri.toLocalFile());
            if(!fileReader.exists()) {
                qDebug() << "Assets : cannot create asset, source URI file does not exist !";
                return;
            }

            //ext + name
            QFileInfo fInfo(uri.fileName());
            auto ext = fInfo.suffix();
            auto name = fInfo.baseName();
            
            //hash
            auto hash = _getFileHash(fileReader);

            //move
            auto expectedStoragePath = _getFilePathToAsset(hash, ext);
            auto success = _moveFileToStore(fileReader, expectedStoragePath);
            if(!success) return;

            //add data
            this->_updateAssetGeometryData(fileReader, ext);
            this->insert(QStringLiteral(u"ext"), ext.toLower());
            this->rename(name);
            this->_setHash(hash);

        }

        static bool _moveFileToStore(QFile &fileReader, const QString &destFilePath) {
            auto success = fileReader.copy(destFilePath);
            if(!success) {
                qDebug() << "Assets : cannot copy asset to storage, it already exists !";
            }
            return success;
        }

        static RPZAsset::Hash _getFileHash(QFile &fileReader) {

            fileReader.open(QFile::ReadOnly);
                
                //read signature...
                RPZAsset::Hash hash = QString::fromUtf8(
                    QCryptographicHash::hash(
                        fileReader.readAll(), 
                        QCryptographicHash::Keccak_224
                    ).toHex()
                );

            fileReader.close();

            return hash;

        }

        static QString _getFilePathToAsset(const RPZAsset::Hash &id, const QString &ext) {
            return QStringLiteral(u"%1/%2.%3")
                            .arg(AppContext::getAssetsFolderLocation())
                            .arg(id)
                            .arg(ext);
        }

};