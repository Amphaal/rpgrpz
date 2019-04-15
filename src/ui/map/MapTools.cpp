#include "MapTools.h"

MapTools::MapTools(QWidget* parent) : QToolBar(parent) {
    
    //self
    this->layout()->setMargin(0);
    this->setMovable(true);
    this->setIconSize(QSize(16,16));
    this->setFloatable(true);

    QObject::connect(
        this, &QToolBar::actionTriggered,
        this, &MapTools::_onToolSelectionChanged
    );

    //selection
    auto selection = new QAction(this);
    selection->setCheckable(true);
    selection->setChecked(true);
    selection->setIcon(QIcon(":/icons/app/tools/cursor.png"));
    selection->setIconText("Selection");
    selection->setData(MapTools::Actions::Select);
    this->addAction(selection);
    QObject::connect(
        selection, &QAction::toggled,
        [&, selection](bool checked) {
            if(!checked && !this->_selectableToolSelected) {
                selection->setChecked(true);
            }
        }
    );
    this->_defaultTool = selection;

    //draw
    this->addSeparator();
    auto draw = new QAction(this);
    draw->setIcon(QIcon(":/icons/app/tools/pen.png"));
    draw->setData(MapTools::Actions::Draw);
    draw->setIconText("Dessiner");
    draw->setCheckable(true);
    this->addAction(draw);

    //draw size
    auto sizer = new QSpinBox(this);
    sizer->setMinimum(1);
    QObject::connect(
        sizer, qOverload<int>(&QSpinBox::valueChanged),
        [&](const int i) {
            emit penSizeChanged(i);
        }
    );
    this->addWidget(sizer);
    this->addSeparator();

    //to north
    auto tonorth = new QAction(this);
    tonorth->setIcon(QIcon(":/icons/app/tools/north.png"));
    tonorth->setData(MapTools::Actions::RotateToNorth);
    tonorth->setIconText("Repositionner vers le nord");
    this->addAction(tonorth);
    
    //reset
    auto reset = new QAction(this);
    reset->setIcon(QIcon(":/icons/app/tools/reset.png"));
    reset->setData(MapTools::Actions::ResetView);
    reset->setIconText("Réinitialiser la vue");
    this->addAction(reset);
    this->addSeparator();

    //selectable tools
    this->_selectableTools.append(draw);
}

void MapTools::unselectAllTools() {
    for(auto action : this->_selectableTools) {
        action->setChecked(false);
    }
    this->_defaultTool->setChecked(true);
}

void MapTools::_onToolSelectionChanged(QAction *action) {

    //if default tool is aimed
    if(action == this->_defaultTool) {
        this->unselectAllTools();
    }

    //define if selectable tool is selected
    this->_selectableToolSelected = false;
    for (auto tool : this->_selectableTools) {
        if(tool->isChecked()) {
            this->_selectableToolSelected = true;
            break;
        }
    }

    //define default tool state
    auto defaultToolState = !this->_selectableToolSelected && !this->_defaultTool->isChecked();
    this->_defaultTool->setChecked(defaultToolState);

    //emit action
    auto mt_action = (MapTools::Actions)action->data().toInt();
    if(!action->isChecked() && _selectableTools.contains(action)) {
        mt_action = MapTools::Actions::None;
    }
    return emit toolSelectionChanged(mt_action);
}