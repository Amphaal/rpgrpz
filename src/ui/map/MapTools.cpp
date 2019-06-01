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
    this->_defaultTool = selection;

    this->addSeparator();

    //text
    auto text = new QAction(this);
    text->setIcon(QIcon(":/icons/app/tools/text.png"));
    text->setData(MapTools::Actions::Text);
    text->setIconText("Ecrire");
    text->setCheckable(true);
    this->addAction(text);
    this->_selectableTools.append(text);

    //draw
    auto draw = new QAction(this);
    draw->setIcon(QIcon(":/icons/app/tools/pen.png"));
    draw->setData(MapTools::Actions::Draw);
    draw->setIconText("Dessiner");
    draw->setCheckable(true);
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

}

void MapTools::selectDefaultTool() {
    this->_unselectAllTools();
    this->_defaultTool->setChecked(true);
}

void MapTools::_unselectAllTools() {
    for(auto action : this->_selectableTools) {
        action->setChecked(false);
    }
}

void MapTools::_onToolSelectionChanged(QAction *action) {

    auto targetAction = (MapTools::Actions)action->data().toInt();

    //define if selectable tool is selected
    this->_selectableToolSelected = false;
    for (auto tool : this->_selectableTools) {
        if(tool->isChecked()) {
            this->_selectableToolSelected = true;
            break;
        }
    }

    this->_unselectAllTools();

    //define default tool state
    if(this->_selectableToolSelected && targetAction != MapTools::Actions::Select) {
        action->setChecked(true);
        this->_defaultTool->setChecked(false);
    } else {
        this->_defaultTool->setChecked(true);
    }

    //emit action
    auto mt_action = action->isChecked() ? targetAction :  MapTools::Actions::None;

    return emit toolSelectionChanged(mt_action);
}