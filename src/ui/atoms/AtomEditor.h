#pragma once

#include <QVBoxLayout>
#include <QWidget>

#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/payloads/Payloads.h"

#include "base/AtomSubEditor.h"
#include "editors/AtomSliderEditor.h"
#include "editors/NonLinearAtomSliderEditor.hpp"

class AtomEditor : public QWidget {

    Q_OBJECT

    public:
        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(QVector<void*> &atomsToBuildFrom);
    
    signals:
        void requiresAtomAlteration(QVariantHash &payload);
    
    private:
        QVector<RPZAtom*> _atoms;
        QHash<RPZAtom::Parameters, AtomSubEditor*> _editorsByParam;

        QHash<RPZAtom::Parameters, QVariant> _findDefaultValuesToBind();
        void _createEditors();

        void _onSubEditorChanged(const RPZAtom::Parameters &parameterWhoChanged, QVariant &value);
};