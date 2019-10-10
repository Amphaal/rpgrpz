#pragma once 

#include <QWidget>
#include <QTabWidget>
#include <QScrollArea>

#include "tabs/LoreTab.hpp"
#include "tabs/StatusTab.hpp"
#include "tabs/InventoriesTab.hpp"

#include "src/shared/models/character/RPZCharacter.hpp"

class CharacterSheet : public QTabWidget {
    
    Q_OBJECT
    
    public:
        CharacterSheet() : 
        _characterTab(new LoreTab), 
        _statusTab(new StatusTab), 
        _inventoriesTab(new InventoriesTab),
        _noteTab(new QTextEdit) {

            this->setVisible(false);

            //tabs
            this->addTab(this->_characterTab, QIcon(":/icons/app/tabs/feather.png"), "Lore");
            this->addTab(_defineScroller(this->_statusTab), QIcon(":/icons/app/tabs/status.png"), "Statut / Compétences");
            this->addTab(this->_inventoriesTab, QIcon(":/icons/app/tabs/chest.png"), "Inventaires");
            this->addTab(this->_noteTab, QIcon(":/icons/app/tabs/noting.png"), "Notes / Autres");

        };

        RPZCharacter generateCharacter() {
            
            auto out = this->_loadedCharacter;
            if(!out.id()) return out;

            //lore
            this->_characterTab->updateCharacter(out);

            //status
            this->_statusTab->updateCharacter(out);

            //inventory
            this->_inventoriesTab->updateCharacter(out);

            //note 
            out.setNotes(this->_noteTab->toPlainText());
            
            return out;
        }

    public slots:
        void loadCharacter(const RPZCharacter& toLoad, bool isReadOnly) {

            //self
            this->_loadedCharacter = toLoad;
            auto characterExists = !toLoad.isEmpty();
            this->setVisible(characterExists);

            //lore
            this->_characterTab->loadCharacter(toLoad);

            //status
            this->_statusTab->loadCharacter(toLoad);

            //inventory
            this->_inventoriesTab->loadCharacter(toLoad);

            //note 
            this->_noteTab->setPlainText(toLoad.notes());

        }

    private:
        RPZCharacter _loadedCharacter;

        LoreTab* _characterTab = nullptr;
        StatusTab* _statusTab = nullptr;
        InventoriesTab* _inventoriesTab = nullptr;
        QTextEdit* _noteTab = nullptr;

        static QScrollArea* _defineScroller(QWidget* toMakeScrollable) {
            auto scroller = new QScrollArea;
            scroller->setFrameShape(QFrame::NoFrame);
            scroller->setWidget(toMakeScrollable);
            scroller->setWidgetResizable(true);
            auto pal = scroller->palette();
            pal.setColor(QPalette::Window, "#FCFCFC");
            scroller->setPalette(pal);
            return scroller;
        }

};
