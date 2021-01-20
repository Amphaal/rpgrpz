// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QPainterPath>
#include <QByteArray>
#include <QVariant>
#include <QVariantList>
#include <QSize>
#include <QFile>
#include <QPointF>

class JSONSerializer {
 public:
    // must cast back to UTF8 to allow correct parsing for Qt's JSON engine
    static QString asBase64(const QPainterPath &path);
    static QString asBase64(QFile &file, int* fileContentSize = nullptr, QString* fileHash = nullptr);

    static QString getFileHash(QFile &file);

    static QPainterPath toPainterPath(const QByteArray &base64);
    static QByteArray toBytes(const QByteArray &base64);

    static QVariant fromQSize(const QSize &size);
    static QSize toQSize(const QVariantList &integerList);

    static QVariant fromPointF(const QPointF &point);
    static QPointF toPointF(const QVariantList &doubleList);

    static QVariant fromPolygons(const QList<QPolygonF> &polys);
    static QList<QPolygonF> toPolygons(const QVariantList &rawPolys);
 private:
    static QString _asBase64(const QByteArray &raw);
    static QString _getFileHash(const QByteArray &raw);
};
