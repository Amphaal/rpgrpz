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
        void signalPayload(AlterationPayload &payload);
    
    signals:
        void requiresAtomAlteration(QVariantHash &payload);
    
    private:
        QVector<RPZAtom*> _atoms;
        void _destroyEditor();

        AtomRotationEditor* _rotateEditor = nullptr;
        AtomScalingEditor* _scaleEditor = nullptr;
};