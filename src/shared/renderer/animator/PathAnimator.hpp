#pragma once

#include <QPropertyAnimation>
#include <QPainterPath>

class PathAnimator : public QPropertyAnimation {
    public:
        PathAnimator(QObject *target, const QByteArray &prop) : QPropertyAnimation(target, prop) {}

        QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const override {
            
            auto fromPath = from.value<QPainterPath>();
            auto toPath = to.value<QPainterPath>();

            QPainterPath interpolatedPath;

            auto stopAt = (int)(toPath.elementCount() * progress);

            for(auto i = 0; i < stopAt; i++) {
                auto element = toPath.elementAt(i);
                interpolatedPath.lineTo(element);
            }

            return QVariant::fromValue<QPainterPath>(interpolatedPath);

        }
};