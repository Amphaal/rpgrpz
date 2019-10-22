#pragma once 

#include <QWidget>
#include <QTabWidget>
#include <QScrollArea>

#include "src/ui/sheets/tabs/LoreTab.hpp"
#include "src/ui/sheets/tabs/StatusTab.hpp"
#include "src/ui/sheets/tabs/InventoriesTab.hpp"

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
            this->addTab(this->_characterTab, QIcon(QStringLiteral(u":/icons/app/tabs/feather.png")), tr("Lore"));
            this->addTab(_defineScroller(this->_statusTab), QIcon(QStringLiteral(u":/icons/app/tabs/status.png")), tr("Status / Abilities"));
            this->addTab(this->_inventoriesTab, QIcon(QStringLiteral(u":/icons/app/tabs/chest.png")), tr("Inventories"));
            this->addTab(this->_noteTab, QIcon(QStringLiteral(u":/icons/app/tabs/noting.png")), tr("Notes / Others"));

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

        bool isReadOnlyMode() const {
            return this->_readOnly;
        }

    public slots:
        void loadCharacter(const RPZCharacter& toLoad, bool isReadOnly) {
            
            this->_readOnly = isReadOnly;

            //self
            this->_loadedCharacter = toLoad;
            auto characterExists = !toLoad.isEmpty();
            this->setVisible(characterExists);

            //lore
            this->_characterTab->loadCharacter(toLoad, isReadOnly);

            //status
            this->_statusTab->loadCharacter(toLoad, isReadOnly);

            //inventory
            this->_inventoriesTab->loadCharacter(toLoad, isReadOnly);

            //note 
            this->setTabEnabled(3, !isReadOnly);
            if(!isReadOnly) {
                this->_noteTab->setPlainText(toLoad.notes());
            } 

        }

    private:
        bool _readOnly = false;
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
