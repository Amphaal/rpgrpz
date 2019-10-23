#pragma once

#include "RPZAsset.hpp"

#include <QByteArray>

class RPZAssetImportPackage : public RPZAsset {
    public:
        RPZAssetImportPackage(const RPZAsset &asset) : RPZAsset(asset) {
            
            //check asset file existance
            auto fp = this->filepath();
            if(fp.isEmpty()) return;

            QFile assetFile(fp);
            assetFile.open(QFile::ReadOnly);
                auto asBase64 = assetFile.readAll().toBase64();
            assetFile.close();

            this->insert(
                QStringLiteral(u"_content"), 
                QString(asBase64)
            );

            this->_isSuccessful = true;

        }

        bool tryIntegratePackage() {
        
            auto bytes = this->assetAsBytes();
            if(!bytes.size() || this->isEmpty()) {
                qWarning() << "Assets : cannot integrate package as it does not contain a raw asset !";
                return false;
            }

            auto success = this->_integrateFrom(
                bytes,
                *this
            );
            
            if(success) this->_clearAssetAsBytes();

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