#pragma once

#include <QSize>
#include <QPointF>
#include <QVariantHash>
#include <QUrl>
#include <QCryptographicHash>

#include "src/helpers/JSONSerializer.h"
#include "src/helpers/_appContext.h"

typedef QString RPZAssetHash; //file hash of the asset

class RPZAsset : public QVariantHash {
    public:
        RPZAsset() {}
        RPZAsset(const QVariantHash &hash) : QVariantHash(hash) {}
        RPZAsset(const QUrl &uri) {
            this->_initFromFile(uri);
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

        static QByteArray assetAsBase64();

    private:
        void _initFromFile(const QUrl &uri) {
            
            QFile fileReader;

            auto success = _createFileHandler(uri, fileReader);
            if(!succcess) return;

            //ext + name
            QFileInfo fInfo(url.fileName());
            auto ext = fInfo.suffix();
            auto name = fInfo.baseName();
            
            //hash
            auto hash = _getFileHash(fileReader);

            //move
            auto expectedStoragePath = _getFilePathToAsset(hash, ext);
            auto success = _moveFileToStore(fileReader, expectedStoragePath);
            if(!succcess) return;

            //add data
            this->insert(QStringLiteral(u"ext"), ext);
            this->insert(QStringLiteral(u"name"), name);
            this->insert(QStringLiteral(u"hash"), hash);

        }

        static bool _moveFileToStore(QFile &fileReader, const QString &destFilePath) {
            auto success = fileReader.copy(destFilePath);
            if(!success) {
                qDebug() << "Assets : cannot copy asset to storage, it already exists !";
            }
        }

        static bool _createFileHandler(const QUrl &url, QFile &fileReader) {
    
            if(!url.isLocalFile()) {
                qDebug() << "Assets : cannot create asset, source URI is not a file !";
                return false;
            }

            //check file exists
            fileReader = QFile(url.toLocalFile());
            if(!fileReader.exists()) {
                qDebug() << "Assets : cannot create asset, source URI file does not exist !";
                return false;
            }

            return true;

        }

        static RPZAssetHash _getFileHash(QFile &fileReader) {

            sourceFile.open(QFile::ReadOnly);
                
                //read signature...
                RPZAssetHash hash = QString::fromUtf8(
                    QCryptographicHash::hash(
                        sourceFile.readAll(), 
                        QCryptographicHash::Keccak_224
                    ).toHex()
                );

            sourceFile.close();

            return hash;

        }

        static QString _getFilePathToAsset(const RPZAssetHash &id, const QString &ext) {
            return QStringLiteral(u"%1/%2.%3")
                            .arg(AppContext::getAssetsFolderLocation())
                            .arg(id)
                            .arg(ext);
        }


};