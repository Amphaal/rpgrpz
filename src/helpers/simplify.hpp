#pragma once

#include <QVector>
#include <QPointF>

class Simplify {
    public:
        static QVector<QPointF> simplify(QVector<QPointF> points, double tolerance = 1.0, bool highestQuality = true) {
            
            if (points.count() <= 2) return points;

            auto sqTolerance = tolerance * tolerance;

            points = highestQuality ? points : _simplifyRadialDist(points, sqTolerance);
            points = _simplifyDouglasPeucker(points, sqTolerance);

            return points;

        }

    private:
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
        static QVector<QPointF> _simplifyRadialDist(const QVector<QPointF> &points, double sqTolerance) {

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

        static void _simplifyDPStep(const QVector<QPointF> &points, int first, int last, double sqTolerance, QVector<QPointF> &simplified) {
            auto maxSqDist = sqTolerance;
            int index;

            for (auto i = first + 1; i < last; i++) {
                auto sqDist = _getSqSegDist(points.at(i), points.at(first), points.at(last));

                if (sqDist > maxSqDist) {
                    index = i;
                    maxSqDist = sqDist;
                }
            }

            if (maxSqDist > sqTolerance) {
                if (index - first > 1) _simplifyDPStep(points, first, index, sqTolerance, simplified);
                simplified.append(points.at(index));
                if (last - index > 1) _simplifyDPStep(points, index, last, sqTolerance, simplified);
            }
            
        }

        // simplification using Ramer-Douglas-Peucker algorithm
        static QVector<QPointF> _simplifyDouglasPeucker(const QVector<QPointF> &points, double sqTolerance) {
            QVector<QPointF> simplified { points.at(0) };
            _simplifyDPStep(points, 0, points.count() - 1, sqTolerance, simplified);
            simplified += points.last();
            return simplified;
        }

};