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
#include <QtMath>

#include "src/helpers/_appContext.h"
#include "src/helpers/JSONSerializer.h"

#include "src/ui/debug/DebugDisplay.hpp"

class RPZMapParameters : public QVariantHash {
 public:
    enum class MovementSystem {
        Grid,
        Linear
    };

    enum class Values {
        MapWidth,
        MinimumZoomScale,
        MaximumZoomScale,
        TileToIngameMeters,
        TileToScreenCentimeters,
        MovementSystem,
        InitialViewPoint
    };

    static inline const QHash<RPZMapParameters::MovementSystem, QString> MSToStr = {
        { RPZMapParameters::MovementSystem::Grid, QT_TRANSLATE_NOOP("QObject", "Grid") },
        { RPZMapParameters::MovementSystem::Linear, QT_TRANSLATE_NOOP("QObject", "Linear") }
    };

    static inline const QHash<RPZMapParameters::MovementSystem, QString> MSToIcon = {
        { RPZMapParameters::MovementSystem::Grid, QStringLiteral(u":/icons/app/tools/grid.png") },
        { RPZMapParameters::MovementSystem::Linear, QStringLiteral(u":/icons/app/tools/ovale.png") }
    };

    static inline const QHash<RPZMapParameters::MovementSystem, QString> MSToDescr = {
        { RPZMapParameters::MovementSystem::Grid, QT_TRANSLATE_NOOP("QObject", "Players can only move from tile to tile") },
        { RPZMapParameters::MovementSystem::Linear, QT_TRANSLATE_NOOP("QObject", "Players can move freely") }
    };

    static inline QHash<RPZMapParameters::Values, QVariant> defaultValues = {
        { RPZMapParameters::Values::MapWidth, (double)36000 },
        { RPZMapParameters::Values::MinimumZoomScale, (double)0.05 },
        { RPZMapParameters::Values::MaximumZoomScale, (double)5 },
        { RPZMapParameters::Values::TileToIngameMeters, (double)1.5 },
        { RPZMapParameters::Values::TileToScreenCentimeters, (double)1.3 },
        { RPZMapParameters::Values::MovementSystem, (int)0 },
        { RPZMapParameters::Values::InitialViewPoint, QPointF() },
    };

    RPZMapParameters() {}
    explicit RPZMapParameters(const QVariantHash &hash) : QVariantHash(hash) {}

    int mapWidthInPoints() const {
        return this->_getParam(RPZMapParameters::Values::MapWidth).toInt();
    }

    const QPointF initialViewPoint() const {
        return JSONSerializer::toPointF(
            this->_getParam(RPZMapParameters::Values::InitialViewPoint).toList()
        );
    }

    const QRectF sceneRect() const {
        auto mapWidthInPoints = (double)this->mapWidthInPoints();

        return QRectF(
            { -mapWidthInPoints / 2, -mapWidthInPoints / 2 },
            QSizeF(mapWidthInPoints, mapWidthInPoints)
        );
    }

    double minimumZoomScale() const {
        return this->_getParam(RPZMapParameters::Values::MinimumZoomScale).toDouble();
    }

    double maximumZoomScale() const {
        return this->_getParam(RPZMapParameters::Values::MaximumZoomScale).toDouble();
    }

    qreal tileWidthInPoints() const {
        return AppContext::pointPerCentimeters() * this->tileToScreenCentimeters();
    }

    const QSizeF tileSizeInPoints() const {
        auto width = this->tileWidthInPoints();
        return QSizeF(width, width);
    }

    double tileToIngameMeters() const {
        return this->_getParam(RPZMapParameters::Values::TileToIngameMeters).toDouble();
    }

    double tileToScreenCentimeters() const {
        return this->_getParam(RPZMapParameters::Values::TileToScreenCentimeters).toDouble();
    }

    double distanceIntoIngameMeters(qreal distanceAsPoints) const {
        auto distanceAsTiles = distanceAsPoints / this->tileWidthInPoints();
        auto meters = distanceAsTiles * this->tileToIngameMeters();
        return meters;
    }

    void alignPointToGridTile(QPointF &toAlignToGrid, QSizeF reference = QSizeF()) const {
        auto typeDifference = this->tileWidthInPoints();

        // prevent zero
        auto x = toAlignToGrid.x();
        auto y = toAlignToGrid.y();
        if(x == 0) x = .01;
        if(y == 0) y = .01;

        // check if corrections need to be made
        if(reference.isNull()) reference = tileSizeInPoints();
        auto mustCorrect = [=](qreal val) {
            auto closest = static_cast<int>(val / typeDifference);
            return (closest % 2) != 0;
        };
        auto mustCorrectX = mustCorrect(reference.width());
        auto mustCorrectY = mustCorrect(reference.height());

        auto xLimitCount = x / typeDifference;
        auto yLimitCount = y / typeDifference;

        if(mustCorrectX) xLimitCount += .5;
        if(mustCorrectY) yLimitCount += .5;

        // define tile rect
        QRectF tile(
            QPointF(qCeil(xLimitCount) * typeDifference, qCeil(yLimitCount) * typeDifference),
            QPointF(qFloor(xLimitCount) * typeDifference, qFloor(yLimitCount) * typeDifference)
        );

        tile.moveTo({
            mustCorrectX ? tile.x() - typeDifference / 2 : tile.x(),
            mustCorrectY ? tile.y() - typeDifference / 2 : tile.y()
        });

        #ifdef _DEBUG
            DebugDisplay::updatePoint(toAlignToGrid);
            DebugDisplay::updateIndex(xLimitCount, yLimitCount);
            DebugDisplay::updateCorrect(mustCorrectX, mustCorrectY);
        #endif

        // update point
        toAlignToGrid = tile.center();
    }

    void alignPointToGridCrossroad(QPointF &scenePos) const {
        auto q = this->tileWidthInPoints();

        auto x = scenePos.x();
        auto y = scenePos.y();

        auto qx = x / q;
        auto qy = y / q;

        auto qxR = qFloor(qx);
        auto qyR = qFloor(qy);

        scenePos = QPointF(qxR * q, qyR * q);
    }

    void alignPointToGrid(QPointF &scenePos) const {
        auto q = this->tileWidthInPoints() / 2;

        auto x = scenePos.x();
        auto y = scenePos.y();

        auto qx = x / q;
        auto qy = y / q;

        auto qxR = qx >= 0 ? qFloor(qx) : qCeil(qx);
        auto qyR = qy >= 0 ? qFloor(qy) : qCeil(qy);

        if (!(qxR % 2)) {
            if (qx >= 0) qxR++;
            else
                qxR--;
        }
        if (!(qyR % 2)) {
            if (qy >= 0) qyR++;
            else
                qyR--;
        }

        scenePos = QPointF(qxR * q, qyR * q);
    }

    RPZMapParameters::MovementSystem movementSystem() const {
        return (RPZMapParameters::MovementSystem)this->_getParam(RPZMapParameters::Values::MovementSystem).toInt();
    }

    void setParameter(const RPZMapParameters::Values &valueType, const QVariant &val) {
        this->insert(_valuesKeys.value(valueType), val);
    }

 private:
    static inline QHash<RPZMapParameters::Values, QString> _valuesKeys = {
            { RPZMapParameters::Values::MapWidth, QStringLiteral(u"size") },
            { RPZMapParameters::Values::MinimumZoomScale, QStringLiteral(u"minZS") },
            { RPZMapParameters::Values::MaximumZoomScale, QStringLiteral(u"maxZS") },
            { RPZMapParameters::Values::TileToIngameMeters, QStringLiteral(u"ttim") },
            { RPZMapParameters::Values::TileToScreenCentimeters, QStringLiteral(u"ttsc") },
            { RPZMapParameters::Values::MovementSystem, QStringLiteral(u"msys") },
            { RPZMapParameters::Values::InitialViewPoint, QStringLiteral(u"ivp") }
        };

    QVariant _getParam(const RPZMapParameters::Values &valType) const {
        return this->value(_valuesKeys.value(valType), defaultValues.value(valType));
    }
};
inline uint qHash(const RPZMapParameters::Values &key, uint seed = 0) {return uint(key) ^ seed;}
inline uint qHash(const RPZMapParameters::MovementSystem &key, uint seed = 0) {return uint(key) ^ seed;}
