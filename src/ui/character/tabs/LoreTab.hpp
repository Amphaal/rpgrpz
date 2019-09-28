#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QValidator>
#include <QBuffer>

#include "src/shared/models/RPZCharacter.hpp"

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
    
        void updateCharacter(RPZCharacter &toUpdate) {

            if(!this->_usingDefaultPortrait) {
                QByteArray bArray;
                QBuffer buffer(&bArray);
                buffer.open(QIODevice::WriteOnly);
                this->_imgLbl->pixmap()->save(&buffer);
                toUpdate.setPortrait(bArray);
            }

            toUpdate.setName(this->_sheetNameEdit->text());
            toUpdate.setArchtype(this->_archtypeEdit->text());
            toUpdate.setDescription(this->_descriptionEdit->toPlainText());
            toUpdate.setStory(this->_storyEdit->toPlainText());
        }   

        void loadCharacter(const RPZCharacter &toLoad) {
            
            auto portraitAsBytes = toLoad.portrait();
            if(!portraitAsBytes.isEmpty()) {
                QPixmap p;
                p.loadFromData(portraitAsBytes);
                this->_imgLbl->setPixmap(p);
                this->_usingDefaultPortrait = false;
            } else {
                this->_imgLbl->setPixmap(*_defaultPortrait);
                this->_usingDefaultPortrait = true;
            }

            this->_sheetNameEdit->setText(toLoad.name());
            this->_archtypeEdit->setText(toLoad.archtype());
            this->_descriptionEdit->setPlainText(toLoad.description());
            this->_storyEdit->setPlainText(toLoad.story());
        }

    private:
        bool _usingDefaultPortrait = true;
        static inline QPixmap* _defaultPortrait = nullptr;

        QLabel* _imgLbl = nullptr;
        QLineEdit* _sheetNameEdit = nullptr;
        QLineEdit* _archtypeEdit = nullptr;
        QTextEdit* _descriptionEdit = nullptr;
        QTextEdit* _storyEdit = nullptr;
};