#include "MapTools.h"

MapTools::MapTools(QWidget* parent) : QToolBar(parent) {
    
    //self
    this->layout()->setMargin(0);
    this->setIconSize(QSize(16, 16));
    this->setMovable(true);
    this->setFloatable(true);

    QObject::connect(
        this, &QToolBar::actionTriggered,
        this, &MapTools::_onToolSelectionChanged
    );

    //selection
    auto selection = RPZActions::selectionTool();
    selection->setChecked(true);
    selection->setData(MapTools::Actions::Select);
    this->addAction(selection);
    this->_defaultTool = selection;

    this->addSeparator();

    //text
    auto text = RPZActions::writeTool();
    text->setData(MapTools::Actions::Text);
    this->addAction(text);
    this->_selectableTools.append(text);

    //draw
    auto draw = RPZActions::drawTool();
    draw->setData(MapTools::Actions::Draw);
    this->addAction(draw);
    this->_selectableTools.append(draw);

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
    
    //reset
    auto reset = RPZActions::resetView();
    reset->setData(MapTools::Actions::ResetView);
    this->addAction(reset);
    this->addSeparator();

}

void MapTools::selectDefaultTool() {
    this->_unselectAllTools();
    this->_defaultTool->setChecked(true);
}

void MapTools::_unselectAllTools() {
    for(auto action : this->actions()) {
        action->setChecked(false);
    }
}

QAction* MapTools::_getCheckedTool() {
    for(auto tool : this->_selectableTools) {
        if(tool->isChecked()) return tool;
    }

    return this->_defaultTool;
}

void MapTools::_onToolSelectionChanged(QAction *action) {

    auto targetAction = (MapTools::Actions)action->data().toInt();
    auto targetIsSelectable = this->_selectableTools.contains(action);
    auto targetIsChecked = action->isChecked();
    auto currentlyChecked = this->_getCheckedTool();

    this->_unselectAllTools();

    if(targetIsSelectable && targetIsChecked) {
        action->setChecked(true);
    } 
    else if(action == this->_defaultTool) {
        this->_defaultTool->setChecked(true);
    } 
    else {
        currentlyChecked->setChecked(true);
    }

    //if unchecked
    if(targetIsSelectable && !targetIsChecked) {
        targetAction = MapTools::Actions::None;
    }

    return emit toolSelectionChanged(targetAction);
}