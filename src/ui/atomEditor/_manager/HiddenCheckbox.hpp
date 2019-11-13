#pragma once

#include <QCheckBox>

#include "src/helpers/_appContext.h"

class HiddenCheckbox : public QCheckBox {
    public:
        HiddenCheckbox(QWidget* parent) : QCheckBox(parent) {
            this->setText(QObject::tr("Hidden"));
            this->setChecked(AppContext::settings()->hiddenAtomAsDefault());
            this->setLayoutDirection(Qt::LeftToRight);
        };
        
        ~HiddenCheckbox() {
            auto val = this->isChecked();
            AppContext::settings()->setHiddenAtomAsDefault(val);
        }

};