#pragma once

#include <QWidget>
#include <QSlider>
#include <QVBoxLayout>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/entities/RPZAtom.h"
#include "src/shared/models/payloads/base/AlterationPayload.hpp"

class AtomSliderEditor : public QWidget {
    public:
        AtomSliderEditor(const QString &descr, const QString &suffix, int minimum, int maximum);

        void loadAtomsAsTemplate(QVector<RPZAtom*> &atoms);
        virtual AlterationPayload createPayload() = 0;

        QSlider* slider();

    protected:
        void _onSliderChanging(int sliderVal);

        virtual double outputValue();
        virtual double atomValue(RPZAtom &atom) = 0;
        virtual int atomValueToSliderValue(RPZAtom &atom) = 0;
        virtual void _updateGraphicsItem(QGraphicsItem* giToUpdate, double value) = 0;

        QVector<snowflake_uid> _atomsToSnowflakeList();

    private:        
        QSlider* _slider = nullptr;
        AtomEditorLineDescriptor* _descr = nullptr;
        QVector<RPZAtom*> _atoms;
        
        RPZAtom atomTemplate();
};