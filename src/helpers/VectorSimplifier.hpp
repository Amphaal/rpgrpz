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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QVector>
#include <QPointF>
#include <QPainterPath>
#include <QPen>

#include "src/_libs/clipper/clipper.hpp"

class VectorSimplifier {
    public:
        static QVector<QPointF> reduce(QVector<QPointF> points, double tolerance = 1.0, bool highestQuality = true) {
            
            if (points.count() <= 2) return points;

            auto sqTolerance = tolerance * tolerance;

            points = highestQuality ? points : _reduceRadialDist(points, sqTolerance);
            points = _reduceDouglasPeucker(points, sqTolerance);

            return points;

        }

        static QList<QPolygonF> simplifyPolygon(const QPolygonF &poly) {
            ClipperLib::Paths outRaw;
            ClipperLib::SimplifyPolygon(_toPath(poly), outRaw, ClipperLib::PolyFillType::pftNonZero);
            return toPolys(outRaw);
        }

        static ClipperLib::Paths toPaths(const QList<QPolygonF> &sourcePolys) {
            ClipperLib::Paths out;
            for(const auto &poly : sourcePolys) {
                out.push_back(_toPath(poly));
            }
            return out;
        }

        static QList<QPolygonF> toPolys(const ClipperLib::Paths &paths) {
            
            QList<QPolygonF> out;
            
            for(const auto &path : paths) {
                
                QPolygonF poly;

                for(const auto &point : path) {
                    poly << QPointF(
                        ((double)point.X) / _precision, 
                        ((double)point.Y) / _precision 
                    );
                }

                out << poly;

            }

            return out;

        }

        static const QPolygonF reducePolygon(const QPolygonF &toreduce) {
            return reduce(toreduce);
        } 

        static const QPainterPath reducePath(const QPainterPath &sourcePath) {
            
            //condense coords
            QVector<QPointF> toReduce;
            
            for(auto i = 0; i < sourcePath.elementCount(); i++) {
                
                auto elem = sourcePath.elementAt(i);
                if(!elem.isLineTo()) continue;

                toReduce.push_back({ elem.x, elem.y });

            }

            //reduce
            auto simplified = reduce(toReduce);
            
            QPainterPath destPath;
            destPath.moveTo(0,0);

            //fill painterpath
            for(auto &point : simplified) {
                destPath.lineTo(point);
            }

            return destPath;

        }

    private:  
        static constexpr int _precision = 3;

        static ClipperLib::Path _toPath(const QPolygonF &sourcePoly) {
            
            ClipperLib::Path rawPath;
            
            for(const auto &point : sourcePoly) {
                
                rawPath.push_back({ 
                    (int)(point.x() * _precision), 
                    (int)(point.y()  * _precision)
                });

            }

            return rawPath;

        }

        // square distance between 2 points
        static double _getSqDist(const QPointF &p1, const QPointF &p2) {

            auto dx = p1.x() - p2.x();
            auto dy = p1.y() - p2.y();

            return dx * dx + dy * dy;

        }

        // square distance from a point to a segment
        static double _getSqSegDist(const QPointF &p, const QPointF &p1, const QPointF &p2) {

            auto x = p1.x();
            auto y = p1.y();
            auto dx = p2.x() - x;
            auto dy = p2.y() - y;

            if (dx != 0 || dy != 0) {

                auto t = ((p.x() - x) * dx + (p.y() - y) * dy) / (dx * dx + dy * dy);

                if (t > 1) {
                    x = p2.x();
                    y = p2.y();

                } else if (t > 0) {
                    x += dx * t;
                    y += dy * t;
                }
            }

            dx = p.x() - x;
            dy = p.y() - y;

            return dx * dx + dy * dy;
        }

        // basic distance-based simplification
        static QVector<QPointF> _reduceRadialDist(const QVector<QPointF> &points, double sqTolerance) {

            auto prevPoint = points.first();
            QVector<QPointF> newPoints {prevPoint};
            QPointF point;

            for (auto i = 1; i < points.count(); i++) {
                point = points.at(i);

                if (_getSqDist(point, prevPoint) > sqTolerance) {
                    newPoints.append(point);
                    prevPoint = point;
                }
            }

            if (prevPoint != point) newPoints.append(point);

            return newPoints;

        }

        static void _reduceDPStep(const QVector<QPointF> &points, int first, int last, double sqTolerance, QVector<QPointF> &simplified) {
            auto maxSqDist = sqTolerance;
            int index = 0;

            for (auto i = first + 1; i < last; i++) {
                auto sqDist = _getSqSegDist(points.at(i), points.at(first), points.at(last));

                if (sqDist > maxSqDist) {
                    index = i;
                    maxSqDist = sqDist;
                }
            }

            if (maxSqDist > sqTolerance) {
                if (index - first > 1) _reduceDPStep(points, first, index, sqTolerance, simplified);
                simplified.append(points.at(index));
                if (last - index > 1) _reduceDPStep(points, index, last, sqTolerance, simplified);
            }
            
        }

        // simplification using Ramer-Douglas-Peucker algorithm
        static QVector<QPointF> _reduceDouglasPeucker(const QVector<QPointF> &points, double sqTolerance) {
            QVector<QPointF> simplified { points.at(0) };
            _reduceDPStep(points, 0, points.count() - 1, sqTolerance, simplified);
            simplified += points.last();
            return simplified;
        }

};