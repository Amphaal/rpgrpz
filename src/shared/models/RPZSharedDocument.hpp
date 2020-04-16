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
#include <QMimeDatabase>

class RPZSharedDocument : public QVariantHash {
    public:
        using FileHash = QString;

        RPZSharedDocument();
        RPZSharedDocument(const QVariantHash &hash) : QVariantHash(hash) {};
        RPZSharedDocument(const QUrl &localFileUrl, const QMimeDatabase &mimeDb) {
            this->_inst(localFileUrl, mimeDb);
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

        QString docMimeType() const {
            return this->value(QStringLiteral(u"mime")).toString(); 
        }

    private:
        bool _localInstSuccess = false;

        void _inst(const QUrl &localFileUrl, const QMimeDatabase &mimeDb) {
            
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
            auto mime = mimeDb.mimeTypeForUrl(localFileUrl).name();
            auto name = fi.completeBaseName();

            //insert in obj
            this->insert(QStringLiteral(u"fileH"), hash);
            this->insert(QStringLiteral(u"nm"), name);
            this->insert(QStringLiteral(u"doc"), bytes);
            this->insert(QStringLiteral(u"mime"), mime);

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