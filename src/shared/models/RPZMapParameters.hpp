#pragma once

#include <QVariantHash>
#include <QtMath>

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

        static inline QHash<RPZMapParameters::Values, double> defaultValues = {
            { RPZMapParameters::Values::MapWidth, 36000 },
            { RPZMapParameters::Values::MinimumZoomScale, 0.05 },
            { RPZMapParameters::Values::MaximumZoomScale, 5 },
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

        
        const QSizeF tileSizeInPoints() const {
            auto width = this->tileWidthInPoints();
            return QSizeF(width, width);
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
        
        void alignPointToGridCenter(QPointF &scenePos) const {
                        
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

                if(!(qxR % 2)) {
                    if(qx >= 0) qxR++;
                    else qxR--;
                }
                if(!(qyR % 2)) {
                    if(qy >= 0) qyR++;
                    else qyR--;
                }

                scenePos = QPointF(qxR * q, qyR * q);

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
