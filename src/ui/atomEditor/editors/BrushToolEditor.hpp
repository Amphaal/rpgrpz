#pragma once

#include "src/ui/atomEditor/base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QComboBox>

#include "src/ui/map/MapView.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/editors/AtomSliderEditor.h"

class BrushToolEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        static inline QMap<int, QString> _strBT {
            { (int)BrushType::Stamp, QT_TR_NOOP("Stamp") },
            { (int)BrushType::Rectangle, QT_TR_NOOP("Rectangle") },
            { (int)BrushType::Ovale, QT_TR_NOOP("Ovale") },
            { (int)BrushType::RoundBrush, QT_TR_NOOP("Round brush") },
            { (int)BrushType::Cutter, QT_TR_NOOP("Cutter") },
            { (int)BrushType::Scissors, QT_TR_NOOP("Scissors") }
        };

        static inline QHash<int, QString> _BTicons {
            { (int)BrushType::Stamp, ":/icons/app/tools/stamp.png" },
            { (int)BrushType::Rectangle, ":/icons/app/tools/rectangle.png" },
            { (int)BrushType::Ovale, ":/icons/app/tools/ovale.png" },
            { (int)BrushType::RoundBrush, ":/icons/app/tools/roundBrush.png" },
            { (int)BrushType::Cutter, ":/icons/app/tools/cutter.png" },
            { (int)BrushType::Scissors, ":/icons/app/tools/scissors.png" }
        };

        QComboBox* _combo = nullptr;

    public:
        BrushToolEditor() : AtomSubEditor(AtomParameter::BrushStyle) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            for(auto i = _strBT.constBegin(); i != _strBT.constEnd(); i++) {

                auto tool = i.key();
                auto translatedName = tr(i.value().toStdString().c_str());
                auto associatedIcon = QIcon(_BTicons[tool]);

                this->_combo->addItem(associatedIcon, translatedName, tool);

            }

            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    auto out = QVariant(currentIndex);
                    emit valueConfirmedForPayload(this->_param, out);
                }
            );

            this->layout()->addWidget(this->_combo);
        };

        QVariant loadTemplate(const AtomUpdates &defaultValues, bool updateMode) override {
            
            auto defaultValue = AtomSubEditor::loadTemplate(defaultValues, updateMode);

            QSignalBlocker b(this->_combo);
            auto indexToSelect = this->_combo->findData(defaultValue);
            this->_combo->setCurrentIndex(indexToSelect);

            this->_combo->setEnabled(!updateMode);

            return defaultValue;
            
        }

};