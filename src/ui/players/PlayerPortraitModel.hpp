#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QIcon>

#include "src/shared/models/character/RPZCharacter.hpp"

class PlayerPortraitModel : public QAbstractListModel {
    public:
        PlayerPortraitModel(const QVector<RPZCharacter> &toDisplay) : _charactersToDisplay(toDisplay) { };

        QVariant data(const QModelIndex &index, int role) const override {
            
            if (!index.isValid()) return QVariant();

            auto targeted = this->_charactersToDisplay.value(index.row);
            if(targeted.isEmpty()) return QVariant();

            switch(role) {
                
                case Qt::DecorationRole: {
                    return QIcon(targeted.portrait());
                }
                break;

                case Qt::DisplayRole: {
                    return targeted.name();
                }
                break; 

            }

        }


    private:
        QVector<RPZCharacter> _charactersToDisplay;
};