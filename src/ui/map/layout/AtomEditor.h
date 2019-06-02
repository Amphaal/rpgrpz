#include <QGroupBox>
#include <QVBoxLayout>

#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/models/Payloads.h"

class AtomEditorLineDescriptor : public QWidget {
    public:
        AtomEditorLineDescriptor(const QString &description, const QString &suffix, QWidget* parent = nullptr);
        void updateValue(double value);
        void cannotDisplayValue();
    
    private:
        QString _suffix;
        QLabel* _valLbl = nullptr;
};

class AtomEditor : public QGroupBox {

    Q_OBJECT

    public:
        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(QVector<void*> &atoms);
    
    signals:
        void requiresAtomAlteration(QVariantHash &payload);
    
    private:
        QVector<RPZAtom*> _atoms;
        void _destroyEditor();

        AtomEditorLineDescriptor* _rotateDescr = nullptr;
        QSlider* _rotateSlider = nullptr;

        AtomEditorLineDescriptor* _scaleDescr = nullptr;
        QSlider* _scaleSlider = nullptr;

        void _onScaleChanged();
        void _onRotateChanged();

        void _onScaleChanging(int sliderVal);
        void _onRotateChanging(int sliderVal);

        QVector<QUuid> _atomsToUuidList();

        double _toScaleValue(int sliderVal);
        int _fromScaleValue(double scaled);
        static int inline _minScaleSlider = 1;
        static int inline _maxScaleSlider = 1000;
};