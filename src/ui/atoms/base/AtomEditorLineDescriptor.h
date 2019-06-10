#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>

class AtomEditorLineDescriptor : public QWidget {
    public:
        AtomEditorLineDescriptor(const QString &description, const QString &suffix);
        void updateValue(double value);
        void cannotDisplayValue();
    
    private:
        QString _suffix;
        QLabel* _valLbl = nullptr;
};