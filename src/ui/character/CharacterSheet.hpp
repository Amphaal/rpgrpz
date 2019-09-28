#pragma once 

#include <QWidget>
#include <QTabWidget>

#include "tabs/LoreTab.hpp"
#include "tabs/StatusTab.hpp"
#include "tabs/InventoriesTab.hpp"

#include "src/shared/models/RPZCharacter.hpp"

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
            this->addTab(this->_statusTab, QIcon(":/icons/app/tabs/status.png"), "Statut / Compétences");
            this->addTab(this->_inventoriesTab, QIcon(":/icons/app/tabs/chest.png"), "Inventaires");
            this->addTab(this->_noteTab, QIcon(":/icons/app/tabs/noting.png"), "Notes");

        };

    public slots:
        void loadCharacter(const RPZCharacter &toLoad) {
            auto characterExists = !toLoad.isEmpty();
            this->setVisible(characterExists);
        }

    private:
        LoreTab* _characterTab = nullptr;
        StatusTab* _statusTab = nullptr;
        InventoriesTab* _inventoriesTab = nullptr;
        QTextEdit* _noteTab = nullptr;

};
