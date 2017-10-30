#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QObject>
#include <QImage>

#include <QPainter>
#include <QPaintEvent>
#include <QLabel>


class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    QImage m_curImage;
signals:

public slots:
    void onGetImage(QImage img);
};

#endif // IMAGELABEL_H
