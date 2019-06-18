#include "RandomColor.h"

QColor RandomColor::getRandomColor() {
    
    if(!_rand) {
        _rand = QRandomGenerator::global()->bounded(1.0);
    } else {
        _rand = fmod(_rand + _goldenRatio, 1);
    }

    return QColor::fromHsvF(_rand, .5, .95);
}

QList<QColor> RandomColor::generateColors(int howMany) {
    QList<QColor> list;

    for(int i = 0; i < howMany; i++) {
        auto color = getRandomColor();
        list.append(color);
    }   

    return list;
} 