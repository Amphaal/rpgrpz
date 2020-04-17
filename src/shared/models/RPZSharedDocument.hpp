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
// different license and copyright still refer to this GNU General Public License.

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

        RPZSharedDocument();
        RPZSharedDocument(const QVariantHash &hash) : QVariantHash(hash) {};
        RPZSharedDocument(const QUrl &localFileUrl) {
            this->_inst(localFileUrl);
        }
        
        static RPZSharedDocument::NamesStore toNamesStore(const QVariantHash &hash) {

            RPZSharedDocument::NamesStore out;

            for(auto i = hash.begin(); i != hash.end(); i++) {
                
                auto &hash = i.key();
                auto name = i.value().toString();

                out.insert(hash, name);

            }

            return out;

        }

        static QVariantHash toVariantNamesStore(const RPZSharedDocument::Store &store) {
            
            QVariantHash out;

            for(auto i = store.begin(); i != store.end(); i++) {
                
                auto &hash = i.key();
                auto name = i.value().documentName();

                out.insert(hash, name);

            }

            return out;

        }

        QString documentName() const {
            return this->value(QStringLiteral(u"nm")).toString();
        }

        QByteArray document() const {
            return this->value(QStringLiteral(u"doc")).toByteArray();
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
            if(opened) {
                fHandler.write(this->document());
                fHandler.close();
            }

            return filePath;

        }

    private:
        bool _localInstSuccess = false;

        void _inst(const QUrl &localFileUrl) {
            
            //check if url is local
            if(!localFileUrl.isLocalFile()) {
                qDebug() << qUtf8Printable(QStringLiteral("File Share : %1 is not a local file !").arg(localFileUrl.toString()));
                return;
            }
            
            //check if file exists
            auto fullPath = localFileUrl.toLocalFile();
            QFileInfo fi(fullPath);
            
            if(!fi.exists()) {
                qDebug() << qUtf8Printable(QStringLiteral("File Share : %1 does not exist anymore !").arg(localFileUrl.toString()));
                return;
            }

            //read file
            QFile reader(fullPath);
            reader.open(QFile::ReadOnly);
                auto bytes = reader.readAll();
            reader.close();

            //override file if exists
            auto hash = RPZSharedDocument::_getFileHash(bytes);
            auto ext = fi.suffix();
            auto name = fi.completeBaseName();

            //insert in obj
            this->insert(QStringLiteral(u"fileH"), hash);
            this->insert(QStringLiteral(u"nm"), name);
            this->insert(QStringLiteral(u"doc"), bytes);
            this->insert(QStringLiteral(u"ext"), ext);

            //
            this->_localInstSuccess = true;

        }
    
        static RPZSharedDocument::FileHash _getFileHash(const QByteArray &fileBytes) {
                
            //read signature...
            auto hash = QString::fromUtf8(
                QCryptographicHash::hash(
                    fileBytes, 
                    QCryptographicHash::Keccak_224
                ).toHex()
            );

            return hash;

        }

};