#pragma once

#include <QPainter>
#include <QComboBox>

class RPZTestable {

    protected:
        QPainter::CompositionMode _testMode = (QPainter::CompositionMode)0;
        void _initTestCombo() {
            
            this->_testCb = new QComboBox;
            
            auto addOp = [=](QPainter::CompositionMode mode, QString str) {
                this->_testCb->addItem(str, (int)mode);
            };

            addOp(QPainter::CompositionMode_SourceOver, "SourceOver");
            addOp(QPainter::CompositionMode_DestinationOver, "DestinationOver");
            addOp(QPainter::CompositionMode_Clear, "Clear");
            addOp(QPainter::CompositionMode_Source, "Source");
            addOp(QPainter::CompositionMode_Destination, "Destination");
            addOp(QPainter::CompositionMode_SourceIn, "SourceIn");
            addOp(QPainter::CompositionMode_DestinationIn, "DestinationIn");
            addOp(QPainter::CompositionMode_SourceOut, "SourceOut");
            addOp(QPainter::CompositionMode_DestinationOut, "DestinationOut");
            addOp(QPainter::CompositionMode_SourceAtop, "SourceAtop");
            addOp(QPainter::CompositionMode_DestinationAtop, "DestinationAtop");
            addOp(QPainter::CompositionMode_Xor, "Xor");
            addOp(QPainter::CompositionMode_Plus, "Plus");
            addOp(QPainter::CompositionMode_Multiply, "Multiply");
            addOp(QPainter::CompositionMode_Screen, "Screen");
            addOp(QPainter::CompositionMode_Overlay, "Overlay");
            addOp(QPainter::CompositionMode_Darken, "Darken");
            addOp(QPainter::CompositionMode_Lighten, "Lighten");
            addOp(QPainter::CompositionMode_ColorDodge, "ColorDodge");
            addOp(QPainter::CompositionMode_ColorBurn, "ColorBurn");
            addOp(QPainter::CompositionMode_HardLight, "HardLight");
            addOp(QPainter::CompositionMode_SoftLight, "SoftLight");
            addOp(QPainter::CompositionMode_Difference, "Difference");
            addOp(QPainter::CompositionMode_Exclusion, "Exclusion");

            QObject::connect(
                this->_testCb, QOverload<int>::of(&QComboBox::highlighted),
                [=](int index) {
                    this->_testMode = (QPainter::CompositionMode)this->_testCb->itemData(index).toInt();
                }
            );

            this->_testCb->show();

        }

    private:
        QComboBox* _testCb = nullptr;
        

};