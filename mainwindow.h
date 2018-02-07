#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintDeviceWindow>
#include <QPaintEngine>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QTabletEvent>
#include <QDebug>

#include <epframebuffer.h>

#include "canvas.h"
#include "staticdata.h"


class Canvas;
class UpdateObject;

namespace Ui {
class MainWindow;
}

struct TabletData {
    QPointF pos;
    qreal pressure;
    qreal rotation;
    qreal tilt;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual void tabletEvent(QTabletEvent *) override;
    virtual void keyPressEvent(QKeyEvent * event) override;
    virtual void touchEvent(QTouchEvent *);


private:
    Ui::MainWindow *ui;
    QImage *image;
    struct TabletData currentTabData;
    struct TabletData lastTabData;
    bool onStroke;
    QRect drawn;
    Canvas *canvas;
    EPFrameBuffer::WaveformMode drawingMode;

    void draw();
    int mod(int x, int y);
};

#endif // MAINWINDOW_H
