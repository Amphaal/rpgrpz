// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QComboBox>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/_others/ConnectivityObserver.h"
#include "src/shared/hints/HintThread.hpp"

class CharacterPickerEditor : public AtomSubEditor, public ConnectivityObserver {
    Q_OBJECT

 public:
    struct DefaultCharacterSelection {
        RPZCharacter::Id characterId = 0;
        QString characterName;
        QColor characterColor;
    };

    CharacterPickerEditor() : AtomSubEditor({RPZAtom::Parameter::CharacterId, RPZAtom::Parameter::CharacterName, RPZAtom::Parameter::DefaultPlayerColor}, false) {
        this->setVisible(false);

        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

        this->_combo = new QComboBox;

        QObject::connect(
            this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CharacterPickerEditor::_onComboValueChanged
        );

        this->layout()->addWidget(this->_combo);
    }

    void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
        this->_restrictToBoundCharacters = context.representedTypes.contains(RPZAtom::Type::Player);
        AtomSubEditor::loadTemplate(defaultValues, context);
        this->_fillComboDefaultValues(defaultValues, context);
    }

 protected:
    void connectingToServer() override {
        QObject::connect(
            this->_rpzClient, &RPZClient::userDataChanged,
            this, &CharacterPickerEditor::_mayUpdateCharacter
        );

        QObject::connect(
            this->_rpzClient, &RPZClient::charactersCountChanged,
            this, &CharacterPickerEditor::_onCharacterCountChange
        );
    }

    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override {
        this->_availableCharacters.clear();
        this->_updateComboFromAvailableCharacters();
    }

 private:
    QList<RPZCharacter::UserBound> _availableCharacters;
    DefaultCharacterSelection _defaultCharacter;
    bool _restrictToBoundCharacters = false;

    void _updateComboFromAvailableCharacters() {
        // restrict 1 player by PC, but not for other types (NPC typically)
        auto fillWith = _restrictToBoundCharacters ?
            HintThread::hint()->findUnboundCharacters(this->_availableCharacters) :
            this->_availableCharacters;

        // clear combo
        QSignalBlocker b(this->_combo);
        this->_combo->clear();

        // if combo is disabled, just skip
        if (!this->_combo->isEnabled()) {
            this->_combo->addItem(tr("[Cannot configure...]"), 0);
            return;
        } else {  // add default empty option
            this->_combo->addItem(tr("[No character associated]"), 0);
        }

        // fill
        for (const auto &bond : fillWith) {
            const auto &character = bond.second;
            const auto &color = bond.first;

            DefaultCharacterSelection sel { character.id(), character.toString(), color };
            this->_addCharacterItem(sel);
        }

        // find default
        auto indexFound = this->_findItemIndexFromId(this->_defaultCharacter.characterId);

        // found !
        if (indexFound > -1) {
            this->_combo->setCurrentIndex(indexFound);  // define as default
        } else {  // not found...
            this->_addCharacterItem(this->_defaultCharacter, true);
        }
    }

    int _findItemIndexFromId(const RPZCharacter::Id &id) {
        return this->_combo->findData(QVariant::fromValue<RPZCharacter::Id>(id));
    }

    void _addCharacterItem(const DefaultCharacterSelection &characterDescr, bool selected = false) {
        this->_combo->addItem(
            QIcon(":/icons/app/connectivity/cloak.png"),
            characterDescr.characterName,
            QVariant::fromValue<RPZCharacter::Id>(characterDescr.characterId)
        );

        auto insertedAt = this->_combo->count() - 1;

        this->_combo->setItemData(
            insertedAt,
            QVariant::fromValue(characterDescr),
            257
        );

        if (selected) {
            this->_combo->setCurrentIndex(insertedAt);
        }
    }

    QComboBox* _combo = nullptr;
    void _fillComboDefaultValues(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
        // define combo state
        auto allows = context.mode == AtomSubEditor::EditMode::Selection && context.numberOfItems == 1;
        this->setEnabled(allows);

        // check for a bound characterId
        auto characterId = defaultValues.value(RPZAtom::Parameter::CharacterId).toULongLong();

        // define default character
        if (characterId) {
            auto name = defaultValues.value(RPZAtom::Parameter::CharacterName).toString();
            auto color = defaultValues.value(RPZAtom::Parameter::DefaultPlayerColor).value<QColor>();
            this->_defaultCharacter = { characterId, name, color };
        } else {  // reset default character
            this->_defaultCharacter = {};
        }

        // fill combo
        this->_updateComboFromAvailableCharacters();
    }

    void _onComboValueChanged(int index) {
        auto sel = this->_combo->currentData(257).value<DefaultCharacterSelection>();

        emit valueConfirmedForPayload({
            { RPZAtom::Parameter::CharacterId, QString::number(sel.characterId) },  // must be saved as string to prevent JSON parser lack of precision on double conversion
            { RPZAtom::Parameter::CharacterName, sel.characterName },
            { RPZAtom::Parameter::DefaultPlayerColor, sel.characterColor }
        });
    }

    void _mayUpdateCharacter(const RPZUser &changed) {
        // prevent update if not player
        if (changed.role() != RPZUser::Role::Player) return;

        // search for character in combo
        auto character = changed.character();

        // find index of character
        auto index = this->_findItemIndexFromId(character.id());
        if (index == -1) return;

        // update name
        DefaultCharacterSelection sel { character.id(), character.toString(), changed.color() };

        this->_combo->setItemText(index, sel.characterName);
        this->_combo->setItemData(index, QVariant::fromValue(sel), 257);
    }

    void _onCharacterCountChange() {
        this->_availableCharacters = this->_rpzClient->sessionCharacters();
        this->_updateComboFromAvailableCharacters();
    }
};

Q_DECLARE_METATYPE(CharacterPickerEditor::DefaultCharacterSelection);
