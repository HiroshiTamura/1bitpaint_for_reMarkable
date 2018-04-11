#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QImage>
#include <QPainter>

#include <math.h>

#include "mainwindow.h"
#include "staticdata.h"

struct TabletData;


struct PenParam{
    int size;
    int hard;
};

class UpdateObject
{
public:
    UpdateObject( QImage *bmp ,QRect *rect);
    ~UpdateObject();

    QImage *m_bmp;
    QRect *m_rect;

};

class Canvas : public QObject
{
Q_OBJECT
public:
    explicit Canvas( int x ,int y ,QObject *parent = 0);
    ~Canvas();
    UpdateObject *openImage( const QString &name);
    void clear();
    QRect draw(const struct TabletData &lastPoint, const struct TabletData &currentPoint);
    UpdateObject *reduce( QRect rect);
    bool Modified();
    void save( const QString &name);
    void load( const QString &name);
    QImage *getPrev( QRect rect);
    void updatePrev( QRect rect);
    //void undo( History &history);
    //void redo( History &history);
    void setDrawingMode(EPFrameBuffer::WaveformMode drawingMode);
    void togglePen();


    static const int numColors = 17;
    static const int palleteGray[numColors];
    static const int palleteMono[numColors];

signals:
    void saveFile( QImage &);

public slots:

private:
    constexpr static const qreal gamma = 2.2f;
    QImage *bmp;
    QImage *prev;
    bool BmpModified;
    int penIndex;
    struct PenParam pen[2];
    unsigned int color;
    EPFrameBuffer::WaveformMode drawingMode;
    const float MIN_SIZE = .7f;

    void monoImageCopy( QImage &dstImage ,QRect dstRect ,QImage &srcImage ,QRect srcRect);
    QImage *createMonoImage( QImage &srcImage ,QRect rect);
    QRect drawPen(const struct TabletData &lastPoint, const struct TabletData &currentPoint);
    QRect drawPencil(const struct TabletData &lastPoint, const struct TabletData &currentPoint);
    //QRect drawFixedPenWidth( const struct TabletData &lastPoint, const struct TabletData &currentPoint);
    //QRect drawEraser( const struct TabletData &lastPoint, const struct TabletData &currentPoint);


    int DrawCanvasCircle(float x, float y, float r);
    int DrawCanvasHLine(int x1, int x2, int y);
    int DrawCanvasPolygon(QPointF *pt ,size_t num);
    int DrawCanvasLine(float x1 ,float y1 ,float x2 ,float y2);
    int DrawCanvasEllipse(float x, float y, float a, float b, float angle, float strength);
    float distance( const QPointF p1 ,const QPointF p2);
    int DrawCanvasPolygonWithStrength(QPointF *pt, size_t num, float *ptStrength);
    int DrawCanvasHLineWithStrength(int x1, int x2, int y, float strengthL, float strengthR);

    unsigned int rand_xor128();
    int mod(int x, int y);

};

#endif // CANVAS_H
