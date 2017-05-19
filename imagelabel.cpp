#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget* parent)
    :QLabel(parent){

}

ImageLabel::~ImageLabel(){

}

void ImageLabel::mousePressEvent(QMouseEvent *e){
    emit mouseClicked(e);
}

void ImageLabel::mouseMoveEvent(QMouseEvent *e){
    emit mouseMoved(e);
}
