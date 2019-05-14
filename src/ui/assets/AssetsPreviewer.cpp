#include "AssetsPreviewer.h"

AssetsPreviewer::AssetsPreviewer(QWidget * parent) : QGroupBox(parent) {
    this->setTitle("Aperçu");
    this->setAlignment(Qt::AlignHCenter);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    this->setMinimumHeight(250);

    this->setLayout(new QVBoxLayout);
}

void AssetsPreviewer::resetPreview() {
    if(this->_previewedImage) {
        this->layout()->removeWidget(this->_previewedImage);
        delete this->_previewedImage;
        this->_previewedImage = nullptr;
    }
}

void AssetsPreviewer::previewFile(QString localFilePath) {

    //remove current
    this->resetPreview();

    //find file ext
    QFileInfo fi(localFilePath);
    if(!fi.exists()) return;
    auto ext = fi.suffix();

    this->_previewedImage = new QLabel();
    this->_previewedImage->setAlignment(Qt::AlignCenter);

    //switch...
    auto image = QImage();
    if(ext == "svg") {
        auto renderer = QSvgRenderer(localFilePath);
        image = QImage(250, 250, QImage::Format_ARGB32);
        image.fill(QColor(255, 255, 255, 0));
        QPainter painter(&image);
        renderer.render(&painter);
    } else {
       image = QImage(localFilePath);
       image = image.scaled(250, 250);
    }

    //update preview
    auto pixmap = QPixmap::fromImage(image);
    this->_previewedImage->setPixmap(pixmap);

    //finally add to view
    this->layout()->addWidget(this->_previewedImage);

}