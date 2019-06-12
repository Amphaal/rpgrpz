#pragma once

#include <QVBoxLayout>
#include <QWidget>

#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/payloads/Payloads.h"

#include "editors/AtomRotationEditor.hpp"
#include "editors/AtomScalingEditor.hpp"
#include "editors/AtomPenWidthEditor.hpp"

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
        AtomPenWidthEditor* _penWidthEditor = nullptr;

        void _onSubEditorChanged(AtomSliderEditor* editor);
};