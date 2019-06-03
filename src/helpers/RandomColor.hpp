#pragma once

#include <QList>
#include <QColor>
#include <QRandomGenerator>

#include <QDebug>

class RandomColor {
    public:
        static QColor getRandomColor() {
            
            if(!_rand) {
               _rand = QRandomGenerator::global()->bounded(1.0);
            } else {
                _rand = fmod(_rand + _goldenRatio, 1);
            }

            return QColor::fromHsvF(_rand, .5, .95);
        }

        static QList<QColor> generateColors(int howMany = 10) {
            QList<QColor> list;

            for(int i = 0; i < howMany; i++) {
                auto color = getRandomColor();
                list.append(color);
            }   

            return list;
        } 

    private:
        static inline double _rand = 0;
        static inline double _goldenRatio = 0.618033988749895;
};