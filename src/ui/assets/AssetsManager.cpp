#include "AssetsManager.h"

AssetsManager::AssetsManager(QWidget * parent) : QWidget(parent), _tree(new AssetsTreeView), _previewer(new AssetsPreviewer) {
    this->setLayout(new QVBoxLayout);
    
    this->layout()->setMargin(5);
    this->layout()->setSpacing(2);

    this->layout()->addWidget(this->_tree);
    this->layout()->addWidget(this->_previewer);

    this->_tree->show();
    this->_tree->expandAll();

    QObject::connect(
        this->_tree, &AssetsTreeView::requestAssetPreview,
        this->_previewer, &AssetsPreviewer::previewFile
    );

    QObject::connect(
        this->_tree, &AssetsTreeView::requestPreviewReset,
        this->_previewer, &AssetsPreviewer::resetPreview
    );
    
}
