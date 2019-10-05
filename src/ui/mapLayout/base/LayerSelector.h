#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include "src/helpers/_appContext.h"

class LayerSelector : public QWidget {
    public:
        LayerSelector(QWidget *parent = nullptr);
        ~LayerSelector();
        QSpinBox* spinbox();

    private:
        QSpinBox* _spinbox = nullptr;
};