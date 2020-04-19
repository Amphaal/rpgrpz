// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

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

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QAction>
#include <QKeyEvent>
#include <QMenu>
#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

#include "src/shared/models/character/RPZCharacter.hpp"

class InventorySlotModal : public QDialog {
    Q_OBJECT

 public:
    InventorySlotModal(const RPZInventory* from, const QVector<RPZInventory*> &targets, const QVector<QPair<int, RPZInventorySlot>> &toAlter)
    : _targets(targets), _targetsCombo(new QComboBox), _recipientCombo(new QComboBox) {
        this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

        for (const auto &slot : toAlter) this->_toBeAltered.insert(slot.second.id(), {slot.first, slot.second});

        QObject::connect(
            this->_targetsCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            [=]() {
                this->_loadHelpersFromTargetInventorySelection();
        });

        {
            QSignalBlocker b(this->_targetsCombo);
            for (const auto target : targets) {
                this->_addInventoryToCombo(this->_targetsCombo, target);
            }
        }

        this->_recipientCombo->setEnabled(false);
        this->_addInventoryToCombo(this->_recipientCombo, from);

        auto lTransfert = new QHBoxLayout;
        lTransfert->setAlignment(Qt::AlignCenter);
        lTransfert->addWidget(this->_recipientCombo);
        lTransfert->addWidget(new QLabel(">>>"));
        lTransfert->addWidget(this->_targetsCombo);

        auto mainLayout = new QVBoxLayout;
        this->setLayout(mainLayout);
        this->_helperLayout = new QVBoxLayout;

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        connect(buttonBox, &QDialogButtonBox::accepted, this, [=]() { this->done(QDialog::Accepted); });
        connect(buttonBox, &QDialogButtonBox::rejected, this, [=]() { this->done(QDialog::Rejected); });

        mainLayout->addLayout(lTransfert, 0);
        mainLayout->addSpacing(15);
        mainLayout->addLayout(this->_helperLayout);
        mainLayout->addSpacing(20);
        mainLayout->addWidget(buttonBox, 0, Qt::AlignRight);
    }

RPZInventory* getSelectedTargetInventory() {
    auto i = this->_targetsCombo->currentIndex();
    if (i >= this->_targets.count()) return nullptr;
    return this->_targets.value(i);
}

 protected:
    QVBoxLayout* _helperLayout = nullptr;
    QMap<QString, QPair<int, RPZInventorySlot>> _toBeAltered;
    virtual void _loadHelpersFromTargetInventorySelection() = 0;

 private:
    QVector<RPZInventory*> _targets;
    QComboBox* _targetsCombo = nullptr;
    QComboBox* _recipientCombo = nullptr;

    void _addInventoryToCombo(QComboBox* target, const RPZInventory* toAdd) {
        target->addItem(QIcon(QStringLiteral(u":/icons/app/other/bag.png")), toAdd->toString());
    }
};

class InventorySlotMoveModal : public InventorySlotModal {
    Q_OBJECT

 public:
    InventorySlotMoveModal(const RPZInventory* from, const QVector<RPZInventory*> &targets,  const QVector<QPair<int, RPZInventorySlot>> &toAlter)
    : InventorySlotModal(from, targets, toAlter) {
        this->_subLayout = new QFormLayout;
        this->_helperLayout->addLayout(this->_subLayout, 0);

        this->setWindowTitle(tr("Move items to inventory"));
        this->_loadHelpersFromTargetInventorySelection();
    }

    QMap<QString, QPair<int, RPZInventorySlot>> toBeAltered() {
        return this->_toBeAltered;
    }

    void _loadHelpersFromTargetInventorySelection() override {
        // clear previous
        while (this->_subLayout->count()) {
            delete this->_subLayout->itemAt(0)->widget();
        }

        auto targetSlots = this->getSelectedTargetInventory()->inventorySlots();

        for (auto i = this->_toBeAltered.begin(); i != this->_toBeAltered.end(); i++) {
            auto nameAsId = i.key();
            auto eqInTarget = targetSlots.value(nameAsId);

            auto NbrInEq = eqInTarget.howMany();
            auto NbrInSource = i.value().second.howMany();

            auto left = tr("• \"%1\" (%2 to move) :").arg(nameAsId).arg(NbrInSource);
            auto right = tr("%1 (Current number) => %2 (New Total)").arg(NbrInEq).arg(NbrInEq + NbrInSource);

            this->_subLayout->addRow(left, new QLabel(right));
        }
    }

 private:
    QFormLayout* _subLayout = nullptr;
};

class InventorySlotSplitModal : public InventorySlotModal {
    Q_OBJECT

 public:
    InventorySlotSplitModal(const RPZInventory* from, const QVector<RPZInventory*> &targets, const QPair<int, RPZInventorySlot> &toAlter)
    : InventorySlotModal(from, targets, {toAlter}), _splitterFrom(new QSpinBox), _movingLbl(new QLabel), _splitterTo(new QSpinBox) {
        this->setWindowTitle(tr("Split item to inventory"));
        this->_maxFrom = this->toBeAltered().second.howMany();

        this->_updateMovingLabel();

        this->_splitterFrom->setMinimum(1);
        this->_splitterFrom->setMaximum(this->_maxFrom);
        this->_splitterFrom->setValue(this->_maxFrom);
        this->_splitterFrom->setToolTip(tr("Current number of items"));

        this->_splitterTo->setEnabled(false);
        this->_splitterTo->setToolTip(tr("Total number of items in dest. inventory"));

        QObject::connect(
            this->_splitterFrom, qOverload<int>(&QSpinBox::valueChanged),
            this, &InventorySlotSplitModal::_onSplitterFromValueChanged
        );

        auto hBox = new QHBoxLayout;
        hBox->addWidget(this->_splitterFrom, 0, Qt::AlignLeft);
        hBox->addWidget(this->_movingLbl, 1, Qt::AlignCenter);
        hBox->addWidget(this->_splitterTo, 0, Qt::AlignRight);

        this->_helperLayout->addLayout(hBox);
        this->_loadHelpersFromTargetInventorySelection();
    }

    QPair<int, RPZInventorySlot>& toBeAltered() {
        return this->_toBeAltered.first();
    }

    void _loadHelpersFromTargetInventorySelection() override {
        auto eqSlotInTarget = this->getSelectedTargetInventory()->inventorySlots().value(
            this->toBeAltered().second.name()
        );

        this->_minTo = eqSlotInTarget.howMany();
        this->_updateSplitterToValue();
    }

    uint newHowMany() {
        return this->_maxFrom - this->_moving;
    }

    uint movingItemsCount() {
        return this->_moving;
    }

 private:
    uint _moving = 0;
    uint _maxFrom = -1;
    uint _minTo = -1;
    QSpinBox* _splitterFrom = nullptr;
    QLabel* _movingLbl = nullptr;
    QSpinBox* _splitterTo = nullptr;

    void _updateMovingLabel() {
        this->_movingLbl->setText(QStringLiteral(u"+%1").arg(this->_moving));
    }

    void _updateSplitterToValue() {
        this->_splitterTo->setValue(this->_minTo + this->_moving);
    }

    void _onSplitterFromValueChanged(int i) {
        this->_moving = this->_maxFrom - i;
        this->_updateMovingLabel();
        this->_updateSplitterToValue();
    }
};
