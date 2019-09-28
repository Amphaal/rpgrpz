#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QValidator>

class LoreTab : public QWidget {
    public:
        LoreTab() {

            //define default portrait
            if(!_defaultPortrait) _defaultPortrait = new QPixmap(":/asset/default.jpg");

            //character tab
            auto characterTabLayout = new QFormLayout;
            this->setLayout(characterTabLayout);
            
                //portrait
                auto pLayout = new QVBoxLayout;
                pLayout->setContentsMargins(0, 0, 0, 5);
                characterTabLayout->addRow(pLayout); 

                    //portrait placeholder
                    this->_imgLbl = new QLabel;
                    this->_imgLbl->setPixmap(*_defaultPortrait);
                    this->_imgLbl->setMaximumSize(QSize(240, 320));
                    pLayout->addWidget(this->_imgLbl, 0, Qt::AlignCenter);

                    //portrait change button
                    auto changePortraitBtn = new QPushButton("Changer le portrait");
                    pLayout->addWidget(changePortraitBtn, 0, Qt::AlignCenter);

                //character name
                this->_sheetNameEdit = new QLineEdit;
                characterTabLayout->addRow("Nom du personnage :", this->_sheetNameEdit);
                this->_sheetNameEdit->setPlaceholderText(" Nom usuel du personnage [Requis!]");
                this->_sheetNameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\w")));

                //archtype
                this->_archtypeEdit = new QLineEdit;
                this->_archtypeEdit->setPlaceholderText(" Courte et grossière caractérisation du personnage (Paladin Loyal Bon, Chasseur de Prime...)");
                characterTabLayout->addRow("Archétype :", this->_archtypeEdit);

                //character description
                this->_descriptionEdit = new QTextEdit;
                this->_descriptionEdit->setPlaceholderText("Description physique, psychologique, contextuelle...");
                characterTabLayout->addRow("Description :", this->_descriptionEdit);

                //character story
                this->_storyEdit = new QTextEdit;
                this->_storyEdit->setPlaceholderText("Evolution du personnage au cours de ses aventures...");
                characterTabLayout->addRow("Histoire :", this->_storyEdit);
                
        }
    
    private:
        static inline QPixmap* _defaultPortrait = nullptr;

        QLabel* _imgLbl = nullptr;
        QLineEdit* _sheetNameEdit = nullptr;
        QLineEdit* _archtypeEdit = nullptr;
        QTextEdit* _descriptionEdit = nullptr;
        QTextEdit* _storyEdit = nullptr;
};