#pragma once

#include <QTreeWidget>
#include <QKeyEvent>
#include <QTreeWidgetItem>

class RPZTree : public QTreeWidget {
    
    public:
        RPZTree(QWidget * parent) : QTreeWidget(parent) {}
    
    protected:
        void keyPressEvent(QKeyEvent * event) override {

            switch(event->key()) {
                case Qt::Key::Key_Up:
                case Qt::Key::Key_Down:
                    this->_selectNext((Qt::Key)event->key());
                    break;
            }
        }
    
    private:
        void _selectNext(const Qt::Key &key) {
                
                //if no selected item
                if(!this->selectedItems().size()) {
                    auto top = this->topLevelItem(0);
                    if(top) {
                        top->setSelected(true);
                    }
                    return;
                };

                //find selected item
                QTreeWidgetItem* selected = nullptr;
                int n_selector = 0;
                switch(key) {
                    case Qt::Key::Key_Up:
                        selected = this->selectedItems().first();
                        n_selector = -1;
                        break;
                    case Qt::Key::Key_Down:
                        selected = this->selectedItems().last();
                         n_selector = 1;
                        break;
                }

                //find next elem
                auto selected_index = this->indexFromItem(selected, 0);
                auto next_index = selected_index.siblingAtRow(selected_index.row() + n_selector);
                if(!next_index.isValid()) return;

                //select next
                auto next_item = this->itemFromIndex(next_index);
                this->clearSelection();
                this->setItemSelected(next_item, true); 
        };
};