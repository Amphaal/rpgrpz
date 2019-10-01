#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QValidator>
#include <QBuffer>

#include "src/shared/models/character/RPZCharacter.hpp"

class LoreTab : public QWidget {
    public:
        LoreTab() {

            //define default portrait
            if(!_defaultPortrait) _defaultPortrait = new QPixmap(":/asset/default.jpg");

            //character tab
            auto characterTabLayout = new QVBoxLayout;
            this->setLayout(characterTabLayout);
            
                //portrait
                auto pLayout = new QVBoxLayout;
                pLayout->setContentsMargins(0, 0, 0, 5);
                characterTabLayout->addLayout(pLayout); 

                    //portrait placeholder
                    this->_imgLbl = new QLabel;
                    this->_imgLbl->setPixmap(*_defaultPortrait);
                    this->_imgLbl->setMaximumSize(QSize(240, 320));
                    pLayout->addWidget(this->_imgLbl, 0, Qt::AlignCenter);

                    //portrait change button
                    auto changePortraitBtn = new QPushButton("Changer le portrait");
                    pLayout->addWidget(changePortraitBtn, 0, Qt::AlignCenter);
                    QObject::connect(
                        changePortraitBtn, &QPushButton::pressed,
                        this, &LoreTab::_changePortrait
                    );

                //character name
                this->_sheetNameEdit = new QLineEdit;
                this->_sheetNameEdit->setPlaceholderText(" Nom usuel du personnage [Requis!]");
                characterTabLayout->addLayout(_addRow("Nom du personnage :", this->_sheetNameEdit));

                //archtype
                this->_archtypeEdit = new QLineEdit;
                this->_archtypeEdit->setPlaceholderText(" Courte et grossière caractérisation du personnage (Paladin Loyal Bon, Chasseur de Prime...)");
                characterTabLayout->addLayout(_addRow("Archétype :", this->_archtypeEdit));

                //character description
                this->_descriptionEdit = new QTextEdit;
                this->_descriptionEdit->setPlaceholderText("Description physique, psychologique, contextuelle...");
                characterTabLayout->addLayout(_addRow("Description :", this->_descriptionEdit));

                //character story
                this->_storyEdit = new QTextEdit;
                this->_storyEdit->setPlaceholderText("Evolution du personnage au cours de ses aventures...");
                characterTabLayout->addLayout(_addRow("Histoire :", this->_storyEdit), 1);

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

        static QHBoxLayout* _addRow(const QString &descr, QWidget *widget) {
            
            auto l = new QHBoxLayout;
            auto lbl = new QLabel(descr);
            
            lbl->setMinimumWidth(105);

            l->addWidget(lbl);
            l->addWidget(widget, 1);

            return l;
        }

        void _changePortrait() {
            //TODO
        }
};