#pragma once

#include <QList>
#include <QColor>
#include <QRandomGenerator>

#include <QDebug>

class RandomColor {
    public:
        static QColor getRandomColor();

        static QList<QColor> generateColors(int howMany = 10);

    private:
        static inline double _rand = 0;
        static inline const double _goldenRatio = 0.618033988749895;
};