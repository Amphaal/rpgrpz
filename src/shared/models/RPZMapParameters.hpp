#pragma once

#include <QVariantHash>

#include "src/helpers/_appContext.h"

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
            MovementSystem
        };

        static inline const QHash<RPZMapParameters::MovementSystem, QString> MSToStr = {
            { RPZMapParameters::MovementSystem::Linear, QT_TRANSLATE_NOOP("QObject", "Linear") },
            { RPZMapParameters::MovementSystem::Grid, QT_TRANSLATE_NOOP("QObject", "Grid") }
        };

        static inline const QHash<RPZMapParameters::MovementSystem, QString> MSToIcon = {
            { RPZMapParameters::MovementSystem::Linear, QStringLiteral(u":/icons/app/tools/ovale.png") },
            { RPZMapParameters::MovementSystem::Grid, QStringLiteral(u":/icons/app/tools/grid.png") }
        };

        static inline const QHash<RPZMapParameters::MovementSystem, QString> MSToDescr = {
            { RPZMapParameters::MovementSystem::Linear, QT_TRANSLATE_NOOP("QObject", "Players can move freely") },
            { RPZMapParameters::MovementSystem::Grid, QT_TRANSLATE_NOOP("QObject", "Players can only move from tile to tile") }
        };

        static inline QHash<RPZMapParameters::Values, double> defaultValues = {
            { RPZMapParameters::Values::MapWidth, 36000 },
            { RPZMapParameters::Values::MinimumZoomScale, 0.1 },
            { RPZMapParameters::Values::MaximumZoomScale, 100 },
            { RPZMapParameters::Values::TileToIngameMeters, 1.5 },
            { RPZMapParameters::Values::TileToScreenCentimeters, 1.3 },
            { RPZMapParameters::Values::MovementSystem, 0 }
        };

        RPZMapParameters() {};
        explicit RPZMapParameters(const QVariantHash &hash) : QVariantHash(hash) {};

        const int mapWidthInPoints() const {
            return this->_getParam(RPZMapParameters::Values::MapWidth).toInt();
        }

        const QRectF sceneRect() const {

            auto mapWidthInPoints = (double)this->mapWidthInPoints();

            return QRectF(
                { -mapWidthInPoints / 2, -mapWidthInPoints / 2 }, 
                QSizeF(mapWidthInPoints, mapWidthInPoints)
            );

        }

        const double minimumZoomScale() const {
            return this->_getParam(RPZMapParameters::Values::MinimumZoomScale).toDouble();
        }

        const double maximumZoomScale() const {
            return this->_getParam(RPZMapParameters::Values::MaximumZoomScale).toDouble();
        }

        const qreal tileWidthInPoints() const {
            return AppContext::pointPerCentimeters() * this->tileToScreenCentimeters();
        }

        const double tileToIngameMeters() const {
            return this->_getParam(RPZMapParameters::Values::TileToIngameMeters).toDouble();
        };

        const double tileToScreenCentimeters() const {
            return this->_getParam(RPZMapParameters::Values::TileToScreenCentimeters).toDouble();
        }

        const double distanceIntoIngameMeters(qreal distanceAsPoints) const {
            auto distanceAsTiles = distanceAsPoints / this->tileWidthInPoints();
            auto meters = distanceAsTiles * this->tileToIngameMeters();
            return meters;
        }

        const RPZMapParameters::MovementSystem movementSystem() const {
            return (RPZMapParameters::MovementSystem)this->_getParam(RPZMapParameters::Values::MovementSystem).toInt();
        }

        void setParameter(const RPZMapParameters::Values &valueType, double val) {
            this->insert(_valuesKeys.value(valueType), val);
        }
    
    private:
        static inline QHash<RPZMapParameters::Values, QString> _valuesKeys = {
                { RPZMapParameters::Values::MapWidth, QStringLiteral(u"size") },
                { RPZMapParameters::Values::MinimumZoomScale, QStringLiteral(u"minZS") },
                { RPZMapParameters::Values::MaximumZoomScale, QStringLiteral(u"maxZS") },
                { RPZMapParameters::Values::TileToIngameMeters, QStringLiteral(u"ttim") },
                { RPZMapParameters::Values::TileToScreenCentimeters, QStringLiteral(u"ttsc") },
                { RPZMapParameters::Values::MovementSystem, QStringLiteral(u"msys") }
            };
        
        QVariant _getParam(const RPZMapParameters::Values &valType) const {
            return this->value(_valuesKeys.value(valType), defaultValues.value(valType));
        }


};
inline uint qHash(const RPZMapParameters::Values &key, uint seed = 0) {return uint(key) ^ seed;}
inline uint qHash(const RPZMapParameters::MovementSystem &key, uint seed = 0) {return uint(key) ^ seed;}
