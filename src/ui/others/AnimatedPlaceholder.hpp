#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "ClientBindable.hpp"

class AnimatedPlaceholder : public QWidget, public ClientBindable {
    public:
        AnimatedPlaceholder(QWidget* content, QWidget* placeholder) : QWidget(nullptr), 
        content(content),
        placeholder(placeholder), 
        _opacityEffectPlaceholder(new QGraphicsOpacityEffect(this)),
        _opacityEffectContent(new QGraphicsOpacityEffect(this)) {
            
            //default opacity
            this->_opacityEffectPlaceholder->setOpacity(0);
            this->_opacityEffectContent->setOpacity(1);

            //placeholder
            this->placeholder->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            this->placeholder->setLayout(new QVBoxLayout);
            this->placeholder->setGraphicsEffect(this->_opacityEffectPlaceholder);
            
            //content
            this->content->setGraphicsEffect(this->_opacityEffectContent);

            //layout
            this->setLayout(new QVBoxLayout);
            this->layout()->addWidget(this->placeholder);
            this->layout()->addWidget(this->content);

            //layering
            this->placeholder->raise();
            this->content->lower();

        }

        void bindToRPZClient(RPZClient * cc) override {
            ClientBindable::bindToRPZClient(cc);
            this->show();

            QObject::connect(
                cc, &JSONSocket::disconnected,
                [&]() {
                    this->hide();
                }
            );
        }

        void show() {
            this->_doFadeOut(this->_opacityEffectPlaceholder);
            this->_doFadeIn(this->_opacityEffectContent);   
        }

        void hide() {
            this->_doFadeOut(this->_opacityEffectContent);
            this->_doFadeIn(this->_opacityEffectPlaceholder);
        }
    
        QWidget* placeholder;
        QWidget* content;
    
    private:
        QGraphicsOpacityEffect* _opacityEffectPlaceholder;
        QGraphicsOpacityEffect* _opacityEffectContent;
        
        QPropertyAnimation* _getBaseAnimation(QGraphicsOpacityEffect* target) {
            auto anim = new QPropertyAnimation(target, "opacity");
            anim->setDuration(350);
            return anim;
        }

        void _runAnim(QPropertyAnimation* anim) {
            anim->start(QPropertyAnimation::DeleteWhenStopped);
        }
        
        void _doFadeIn(QGraphicsOpacityEffect* target) {
            auto anim = this->_getBaseAnimation(target);
            anim->setStartValue(0);
            anim->setEndValue(1);
            anim->setEasingCurve(QEasingCurve::InBack);
            this->_runAnim(anim);
        }

        void _doFadeOut(QGraphicsOpacityEffect* target) {
            auto anim = this->_getBaseAnimation(target);
            anim->setStartValue(1);
            anim->setEndValue(0);
            anim->setEasingCurve(QEasingCurve::OutBack);
            this->_runAnim(anim);
        }

};