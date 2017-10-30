#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent)
{

}

void ImageLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    if (m_curImage.size().width() <= 0) return;
    //将图像按比例缩放成和窗口一样大小
    QImage img = m_curImage.scaled(this->size(),Qt::IgnoreAspectRatio);
    int x = this->width() - img.width();
    int y = this->height() - img.height();
    painter.drawImage(QPoint(x,y),img); //画出图像
}

void ImageLabel::onGetImage(QImage img)
{
    m_curImage = img;
    this->update();
}


