#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

class NoEditorMessageWidget : public QWidget {
    
    Q_OBJECT

    public:
        NoEditorMessageWidget() {
    
            auto layout = new QVBoxLayout;
            this->setLayout(layout);

            //description
            auto descrLbl = new QLabel(tr("No modifications are possible !"));
            descrLbl->setStyleSheet("font-weight: bold; color: red");
            layout->addWidget(descrLbl, 1, Qt::AlignHCenter);
        }
};