#pragma once

#include <QLayout>
#include <QWidget>

class OverlayingLayout : public QLayout {
    public:
        OverlayingLayout() {
            this->setSpacing(0);
        }
        ~OverlayingLayout() {
            QLayoutItem *item;
            while ((item = takeAt(0)))
                delete item;
        }

        void addItem(QLayoutItem *item) {
            this->_list.append(item);
        }
        QSize sizeHint() const {
            QSize s(0,0);
            int n = this->_list.count();
            if (n > 0)
                s = QSize(100, 70); //start with a nice default size
            int i = 0;
            while (i < n) {
                QLayoutItem *o = this->_list.at(i);
                s = s.expandedTo(o->sizeHint());
                ++i;
            }
            return s + n*QSize(spacing(), spacing());
        }

        QSize minimumSize() const {
            QSize s(0,0);
            int n = this->_list.count();
            int i = 0;
            while (i < n) {
                QLayoutItem *o = this->_list.at(i);
                s = s.expandedTo(o->minimumSize());
                ++i;
            }
            return s + n*QSize(spacing(), spacing());
        }
        
        int count() const {
            return this->_list.size();
        }

        QLayoutItem *itemAt(int idx) const {
            return this->_list.value(idx);
        }

        QLayoutItem *takeAt(int idx){
            return idx >= 0 && idx < this->_list.size() ? this->_list.takeAt(idx) : 0;
        }

        void setGeometry(const QRect &rect) {
            QLayout::setGeometry(rect);

            if (this->_list.size() == 0) return;

            int w = rect.width() - (this->_list.count() - 1) * spacing();
            int h = rect.height() - (this->_list.count() - 1) * spacing();
            int i = 0;

            auto bottomRightAnchor = rect.bottomRight();
            bottomRightAnchor.setX(bottomRightAnchor.x() - 10);
            bottomRightAnchor.setY(bottomRightAnchor.y() - 10);

            while (i < this->_list.size()) {
                
                QLayoutItem *o = this->_list.at(i);    
                auto ww = o->widget()->width();
                auto wh = o->widget()->height(); 

                switch(i) {
                    
                    //map
                    case 0: { 
                        o->setGeometry({0, 0, w, h});
                    }
                    break;

                    //descriptor
                    case 1: { 
                        //let the widget determine its geometry
                    }
                    break;
                    
                    //minimap
                    case 2: { 
                        
                        auto s = bottomRightAnchor;
                        
                        s.setX(s.x() - ww);
                        s.setY(s.y() - wh);
                        
                        QRect geom(s, QSize(ww, wh));

                        o->setGeometry(geom);

                    }
                    break;

                }

                ++i;

            }
        }

    private:
        QList<QLayoutItem*> _list;
};
