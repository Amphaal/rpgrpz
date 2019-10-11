#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QValidator>
#include <QBuffer>
#include <QFileDialog>
#include <QFileInfo>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/helpers/_appContext.h"

class LoreTab : public QWidget {
    public:
        LoreTab() {

            //character tab
            auto characterTabLayout = new QVBoxLayout;
            this->setLayout(characterTabLayout);
            
                //portrait
                auto pLayout = new QVBoxLayout;
                pLayout->setContentsMargins(0, 0, 0, 5);
                characterTabLayout->addLayout(pLayout); 

                    //portrait placeholder
                    this->_imgLbl = new QLabel;
                    this->_imgLbl->setPixmap(RPZCharacter::getDefaultPortrait());
                    this->_imgLbl->setMaximumSize(RPZCharacter::defaultPortraitSize);
                    pLayout->addWidget(this->_imgLbl, 0, Qt::AlignCenter);

                    //portrait change button
                    this->_changePortraitBtn = new QPushButton("Changer le portrait");
                    pLayout->addWidget(this->_changePortraitBtn, 0, Qt::AlignCenter);
                    QObject::connect(
                        this->_changePortraitBtn, &QPushButton::pressed,
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

            if(!this->_customPortrait.isNull()) {
                toUpdate.setPortrait(
                    this->_customPortrait, 
                    this->_customPortraitExt
                );
            }

            toUpdate.setName(this->_sheetNameEdit->text());
            toUpdate.setArchtype(this->_archtypeEdit->text());
            toUpdate.setDescription(this->_descriptionEdit->toPlainText());
            toUpdate.setStory(this->_storyEdit->toPlainText());
        }   

        void loadCharacter(const RPZCharacter &toLoad, bool isReadOnly) {
            
            this->_customPortrait = QPixmap();
            this->_customPortraitExt.clear();

            auto portrait = RPZCharacter::getPortrait(toLoad);
            this->_imgLbl->setPixmap(portrait);

            this->_changePortraitBtn->setVisible(!isReadOnly);

            this->_sheetNameEdit->setText(toLoad.name());
            this->_sheetNameEdit->setReadOnly(isReadOnly);

            this->_archtypeEdit->setText(toLoad.archtype());
            this->_archtypeEdit->setReadOnly(isReadOnly);

            this->_descriptionEdit->setPlainText(toLoad.description());
            this->_descriptionEdit->setReadOnly(isReadOnly);

            this->_storyEdit->setPlainText(toLoad.story());
            this->_storyEdit->setReadOnly(isReadOnly);

        }

    private:
        QPixmap _customPortrait;
        QString _customPortraitExt;


        QPushButton* _changePortraitBtn = nullptr;
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
            
            //get new portrait filepath
            QFileDialog portraitPicker(this, "Changer de portrait", AppContext::getAssetsFolderLocation(), "Images (*.png *.jpg *.jpeg)");
            portraitPicker.setFileMode(QFileDialog::FileMode::ExistingFile);
            portraitPicker.setAcceptMode(QFileDialog::AcceptOpen);
            if(!portraitPicker.exec()) return;
            auto portraitFP = portraitPicker.selectedFiles()[0];
            
            //resize Pixmap and apply
            QPixmap newPortrait(portraitFP);
            this->_customPortrait = newPortrait.scaled(RPZCharacter::defaultPortraitSize);
            this->_imgLbl->setPixmap(this->_customPortrait);

            QFileInfo fi(portraitFP);
            this->_customPortraitExt = fi.suffix();
            
        }
};