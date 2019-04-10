#pragma once

#include <QColor>
#include <QRandomGenerator>

class RandomColor {
    public:
        static QColor getRandomColor() {
            return QColor::fromRgb(QRandomGenerator::global()->generate());
            //_iteration++;
            //auto golden_ratio = fmod(0.618033988749895 * _iteration, 1);
            //return QColor::fromRgb(_rand + golden_ratio);
        }

    //private:
        //static inline int _iteration = -1;
        //static inline auto _rand = QRandomGenerator::global()->generate();
};