#pragma once

#include <QVBoxLayout>
#include <QWidget>

#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/models/Payloads.h"

#include "editors/AtomRotationEditor.hpp"
#include "editors/AtomScalingEditor.hpp"

class AtomEditor : public QWidget {

    Q_OBJECT

    public:
        AtomEditor(QWidget* parent = nullptr);

        void buildEditorFromSelection(QVector<void*> &selectedAtoms);
    
    signals:
        void requiresAtomAlteration(QVariantHash &payload);
    
    private:
        QVector<RPZAtom*> _atoms;

        AtomRotationEditor* _rotateEditor = nullptr;
        AtomScalingEditor* _scaleEditor = nullptr;

        void _onSubEditorChanged(AtomSliderEditor* editor);
};