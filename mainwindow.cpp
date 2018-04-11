#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(1404, 1872);

    //image = new QImage( StaticData::screenWidth ,StaticData::screenHeight ,QImage::Format_ARGB32);
    image = EPFrameBuffer::framebuffer();

    canvas = new Canvas( StaticData::screenWidth ,StaticData::screenHeight ,parent);

    drawingMode = EPFrameBuffer::WaveformMode::Mono;
    canvas->setDrawingMode(drawingMode);

    drawn.setCoords(0 ,0 ,0 ,0);
    onStroke = false;

    //update();
    EPFrameBuffer::clearScreen();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::tabletEvent(QTabletEvent *event)
{
    switch (event->type()) {
        case QEvent::TabletPress:
            //onStroke = true;
            currentTabData.pressure = 0.0f;
            lastTabData = currentTabData;
            drawn.setCoords(0 ,0 ,0 ,0);
            //qDebug() << "tablet press" << drawn.width() << drawn.height();

            break;

        case QEvent::TabletMove:
        {
            //qDebug() << "tablet move";

            //lastTabData = currentTabData;
            //currentTabData.rotation = event->rotation();
            //currentTabData.tilt = event->
            lastTabData = currentTabData;
            currentTabData.pos.setX(event->posF().y() * (( 1404.0f - 348.0f) / 1404.0f));
            currentTabData.pos.setY((1872.0f - event->posF().x() - 476.0f) / ((1872.0f - 476.0f) / 1872.0f));
            currentTabData.pressure = event->pressure();

            qreal tilt = 550;
            qreal rotate = mod(120, 360);
            getRotateAndTilt(rotate, tilt);
            currentTabData.rotation = rotate;
            currentTabData.tilt = tilt;
             //qDebug() << event->posF();

            if(onStroke){
                draw();
            }
            else{
                onStroke = true;
            }
        }
            break;

        case QEvent::TabletRelease:
            //qDebug() << "tablet release";

            currentTabData.pressure = 0.0f;
            lastTabData = currentTabData;

            if( onStroke){
                onStroke = false;

                //EPFrameBuffer::sendUpdate(drawn, EPFrameBuffer::WaveformMode::HighQualityGrayscale, EPFrameBuffer::UpdateMode::FullUpdate, false);
                EPFrameBuffer::sendUpdate(drawn, EPFrameBuffer::WaveformMode::HighQualityGrayscale, EPFrameBuffer::UpdateMode::PartialUpdate, false);
                //qDebug() << drawn;

                //repaint();
                //addHistory();
                drawn.setCoords(0 ,0 ,0 ,0);
            }
            break;

        default:
            break;
    }
}

void MainWindow::getRotateAndTilt(qreal &rotate, qreal &tilt)
{
    int fd = ::open("/dev/input/event0", O_RDWR);

    const int maxEventCount = 64;
    struct input_event event[maxEventCount];
    int eventCount;
    eventCount = read(fd, &event, sizeof(event[0]) * maxEventCount) / sizeof(event[0]);

    static int xTilt;
    static int yTilt;
    for(int i = 0; i < eventCount; i++){
        if(event[i].type == 3){
            if(event[i].code == 26){
                xTilt = event[i].value;
            }
            else if(event[i].code == 27){
                yTilt = event[i].value;
            }
        }
    }

    ::close(fd);

    //qDebug() << "read count:" << eventCount << "  xTilt:" << xTilt << "  yTilt:" << yTilt;



    {
        qreal x, y, z;
        qreal radTiltX, radTiltY;
        qreal radTilt;
        int screenRotateDegrees = 0;

        //qDebug() << event->rotation();
        radTiltX = (qreal)(mod(-xTilt / 100.0f + 90.0f, 360)) / 180.0f * StaticData::PI;
        radTiltY = (qreal)(mod(-yTilt / 100.0f + 90.0f, 360)) / 180.0f * StaticData::PI;

        x = cos(radTiltX);
        y = cos(radTiltY);
        z = 1;

        radTilt = acos(z / sqrt(x*x + y*y + z*z));
        tilt = (90 - radTilt * 180.0 / StaticData::PI) * 10;
        rotate = (atan2(y, x) * 180.0 / StaticData::PI);
        rotate = mod(90 - rotate, 360);

        //qDebug() << "rotate:" << rotate << "  tilt:" << tilt;
    }
}


void MainWindow::touchEvent(QTouchEvent *event)
{
    qDebug() << "touch event";

}

void MainWindow::draw()
{
    QRect rect = canvas->draw(lastTabData, currentTabData);
    if(rect.width() == 1 || rect.height() == 1){
        ;
    }
    else{
        UpdateObject *uo = canvas->reduce(rect);

        QPainter p(image);
        p.drawImage( QPoint( uo->m_rect->left() ,uo->m_rect->top()) ,*uo->m_bmp);
        p.end();

        //EPFrameBuffer::sendUpdate(*uo->m_rect, EPFrameBuffer::WaveformMode::Mono, EPFrameBuffer::UpdateMode::PartialUpdate, false);
        EPFrameBuffer::sendUpdate(*uo->m_rect, drawingMode, EPFrameBuffer::UpdateMode::PartialUpdate, false);

        if(drawn.width() == 1 || drawn.height() == 1){
            drawn = *uo->m_rect;
            //drawn = rect;
        }
        else{
            drawn |= *uo->m_rect;
            //drawn |= rect;
        }
    }
    //qDebug() << rect;

    //update(rect);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    painter.end();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << event->key();
    switch(event->key()){
    case 16777234://left button
        //toggle pen tool
        canvas->togglePen();
        break;

    case 16777232://middle button
        //toggle screen refresh mode
        if(drawingMode != EPFrameBuffer::WaveformMode::Mono){
            drawingMode = EPFrameBuffer::WaveformMode::Mono;
            canvas->setDrawingMode(drawingMode);
        }
        else{
            //reflesh screen
            drawingMode = EPFrameBuffer::WaveformMode::Grayscale;
            canvas->setDrawingMode(drawingMode);

            QRect rect;
            rect.setCoords(0, 0, StaticData::screenWidth * StaticData::scale, StaticData::screenHeight * StaticData::scale);
            UpdateObject *uo = canvas->reduce(rect);

            QPainter p(image);
            p.drawImage( QPoint( uo->m_rect->left() ,uo->m_rect->top()) ,*uo->m_bmp);
            p.end();

            EPFrameBuffer::sendUpdate(*uo->m_rect, EPFrameBuffer::WaveformMode::HighQualityGrayscale, EPFrameBuffer::UpdateMode::FullUpdate, false);

        }

        break;

    case 16777236://right button
        //clear screen
    {
        canvas->clear();

        QRect rect;
        rect.setCoords(0, 0, StaticData::screenWidth * StaticData::scale, StaticData::screenHeight * StaticData::scale);
        UpdateObject *uo = canvas->reduce(rect);

        QPainter p(image);
        p.drawImage( QPoint( uo->m_rect->left() ,uo->m_rect->top()) ,*uo->m_bmp);
        p.end();

        EPFrameBuffer::sendUpdate(*uo->m_rect, EPFrameBuffer::WaveformMode::HighQualityGrayscale, EPFrameBuffer::UpdateMode::FullUpdate, false);
    }
        break;

    default:
        break;
    }

}

int MainWindow::mod(int x, int y)//剰余が負の数になってしまう事への対策
{
    return x%y >= 0 ? x%y : x%y + y;
}
