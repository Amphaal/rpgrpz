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

typedef QString RPZAssetHash; //file hash of the asset

class RPZAsset : public QVariantHash {
    public:
        RPZAsset() {}
        explicit RPZAsset(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZAsset(const QVariantHash &hash, const RPZAssetHash &id) : QVariantHash(hash) {
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

        const QSize shape() const { 
            return JSONSerializer::toQSize(this->value(QStringLiteral(u"shape")).toList()); 
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
        const RPZAssetHash hash() const { 
            return this->value(QStringLiteral(u"hash")).toString(); 
        }

        bool isValidAsset() const {
            return !this->hash().isEmpty();
        }

        const QString filepath() const {
            
            auto expected = _getFilePathToAsset(
                this->hash(),
                this->fileExtension()
            );
            
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

            auto dest = asset.filepath();
            if(dest.isEmpty()) return false;

            QFile writer(dest);
            writer.open(QIODevice::WriteOnly);
                writer.write(assetAsRawBytes);
            writer.close();

            return true;

        }

    private:
        void _setHash(const RPZAssetHash &hash) {
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

        static RPZAssetHash _getFileHash(QFile &fileReader) {

            fileReader.open(QFile::ReadOnly);
                
                //read signature...
                RPZAssetHash hash = QString::fromUtf8(
                    QCryptographicHash::hash(
                        fileReader.readAll(), 
                        QCryptographicHash::Keccak_224
                    ).toHex()
                );

            fileReader.close();

            return hash;

        }

        static QString _getFilePathToAsset(const RPZAssetHash &id, const QString &ext) {
            return QStringLiteral(u"%1/%2.%3")
                            .arg(AppContext::getAssetsFolderLocation())
                            .arg(id)
                            .arg(ext);
        }

};