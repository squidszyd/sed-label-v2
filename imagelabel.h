#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QMouseEvent>

class ImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ImageLabel(QWidget* parent = 0);
    ~ImageLabel();

signals:
    void mouseMoved(QMouseEvent* e);
    void mouseClicked(QMouseEvent* e);

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

};

#endif // IMAGELABEL_H
