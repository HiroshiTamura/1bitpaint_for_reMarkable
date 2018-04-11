#include "canvas.h"

const int Canvas::palleteGray[] = {0, 72, 99, 119, 136, 150, 163, 175, 186, 196, 206, 215, 224, 232, 240, 248, 255 };
const int Canvas::palleteMono[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255};

Canvas::Canvas( int width ,int height ,QObject *parent) :
    QObject(parent)
{
    width = StaticData::screenWidth * StaticData::scale;
    height = StaticData::screenHeight * StaticData::scale;
    bmp = new QImage( width ,height ,QImage::Format_Mono);
    bmp->fill( 0xffffffff);
    prev = new QImage( width ,height ,QImage::Format_Mono);
    //*prev = bmp->copy( 0 ,0 ,width ,height);
    monoImageCopy( *prev ,QRect( 0 ,0 ,width ,height) ,*bmp ,QRect( 0 ,0 ,width ,height));
    BmpModified = false;
    penIndex = 0;
    pen[0].size = 33;
    pen[0].hard = 50;
    pen[1].size = 100;
    pen[1].hard = 50;
    color = 0;
}

Canvas::~Canvas()
{
    delete bmp;
    delete prev;
}

void Canvas::clear()
{
    bmp->fill( 0xffffffff);
}

bool Canvas::Modified()
{
    return BmpModified;
}

UpdateObject *Canvas::openImage( const QString &name)
{
    QRect rt;
    //QString fullPathName = StaticData::imagePath + tr("/") + name;
    int width ,height;

    //QMessageBox::information( 0 ,tr("Debug") ,tr("30"));
    delete bmp;
    delete prev;
    bmp = new QImage( name);
    if( bmp->isNull()){
        //QMessageBox::information( 0 ,tr("Debug") ,tr("31"));
        //delete bmp;
        //QMessageBox::information( 0 ,tr("Debug") ,tr("32"));
        width = StaticData::screenWidth * StaticData::scale;
        height = StaticData::screenHeight * StaticData::scale;
        //QMessageBox::information( 0 ,tr("Debug") ,tr("33"));
        bmp = new QImage( width ,height ,QImage::Format_Mono);
        //QMessageBox::information( 0 ,tr("Debug") ,tr("34"));
        bmp->fill( 0xffffffff);
        //QMessageBox::information( 0 ,tr("Debug") ,tr("35"));
    }
    else{
        //QMessageBox::information( 0 ,tr("Debug") ,tr("36"));
        width = bmp->width();
        //QMessageBox::information( 0 ,tr("Debug") ,tr("37"));
        height = bmp->height();
    }

    prev = new QImage( width ,height ,QImage::Format_Mono);
    //*prev = bmp->copy( 0 ,0 ,width ,height);
    monoImageCopy( *prev ,QRect( 0 ,0 ,width ,height) ,*bmp ,QRect( 0 ,0 ,width ,height));

    BmpModified = false;
    //QMessageBox::information( 0 ,tr("Debug") ,tr("38"));
    rt.setRect( 0 ,0 ,width ,height);
    //QMessageBox::information( 0 ,tr("Debug") ,tr("39"));
    return reduce( rt);
}

void Canvas::save(const QString &name)
{
    bmp->save( name);
}

void Canvas::load(const QString &name)
{
    bmp->load( name);
}

QImage *Canvas::getPrev( QRect rect)
{
    //return prev->copy( rect);
    return createMonoImage( *prev ,rect);
}

void Canvas::updatePrev( QRect rect)
{
    //QPainter p( prev);

    //p.drawImage( rect ,*bmp ,rect);
    monoImageCopy( *prev ,rect ,*bmp ,rect);
}

/*void Canvas::undo( History &history)
{
    //QPainter pBmp( bmp) ,pPrev( prev);
    //QPainter pBmp( bmp);

    //pBmp.drawImage( history.rect ,*history.image);
    monoImageCopy( *bmp ,history.rect ,*history.image ,QRect( 0 ,0 ,history.image->width() ,history.image->height()));
    // *historyImage = prev->copy( rect);
    delete history.image;
    history.image = getPrev( history.rect);
    //historyImage = createMonoImage( *prev ,rectHis);
    //pPrev.drawImage( rect ,*bmp ,rect);
    monoImageCopy( *prev ,history.rect ,*bmp ,history.rect);
}*/

/*void Canvas::redo( History &history)
{
    //QPainter pBmp( bmp);

    //pBmp.drawImage( history.rect ,*history.image);
    monoImageCopy( *bmp ,history.rect ,*history.image ,QRect( 0 ,0 ,history.image->width() ,history.image->height()));
    delete history.image;
    history.image = getPrev( history.rect);
    monoImageCopy( *prev ,history.rect ,*bmp ,history.rect);
}*/

QRect Canvas::draw( const TabletData &lastPoint, const TabletData &currentPoint)
{
    QRect rect;
    switch( penIndex){
    case 0:
        rect = drawPen(lastPoint, currentPoint);
        break;

    case 1:
        rect = drawPencil(lastPoint, currentPoint);
        break;

    case 2:
        rect = drawPen(lastPoint, currentPoint);
        break;

    default:
        rect.setCoords(0,0,0,0);
        break;
    }

    return rect;
}

QRect Canvas::drawPen( const struct TabletData &lastPoint, const struct TabletData &currentPoint)
{
    qreal x2 = (lastPoint.pos.x() * StaticData::scale + .25f);
    qreal y2 = (lastPoint.pos.y() * StaticData::scale + .25f);
    qreal x1 = (currentPoint.pos.x() * StaticData::scale + .25f);
    qreal y1 = (currentPoint.pos.y() * StaticData::scale + .25f);

    QPointF fpoint1( x1 ,y1) ,fpoint2( x2 ,y2);
    QPoint point1( (int)x1 ,(int)y1) ,point2( (int)x2 ,(int)y2);
    qreal gamma;

    if( pen[penIndex].hard >= 50){
         gamma = ((qreal)pen[penIndex].hard - 50) / 50 * 3 + 1;
    }
    else{
        gamma = 1.0f / (((qreal)50 - pen[penIndex].hard) / 50 * 3 + 1);
    }


    qreal size1 = pow(lastPoint.pressure, gamma) * pen[penIndex].size;
    qreal size2 = pow(currentPoint.pressure, gamma) * pen[penIndex].size;

    if( size1 >= 2){
        DrawCanvasCircle( x2 ,y2 ,size2/2);
    }


    if(size1 >=MIN_SIZE || size2 >= MIN_SIZE){
        float x, y;
        if( size1 < MIN_SIZE){//begin stroke
            x = x2 + (x1 - x2) * (MIN_SIZE - size1) / (size2 - size1);
            y = y2 + (y1 - y2) * (MIN_SIZE - size1) / (size2 - size1);
            DrawCanvasLine(x+.5 ,y+.5 ,x1+.5 ,y1+.5);
        }
        else if( size2 < MIN_SIZE){//end stroke
            x = x1 + (x2 - x1) * (MIN_SIZE - size2) / (size1 - size2);
            y = y1 + (y2 - y1) * (MIN_SIZE - size2) / (size1 - size2);
            DrawCanvasLine(x2+.5 ,y2+.5 ,x+.5 ,y+.5);
        }
        else if( size1 <= 1 || size2 <= 1){
            DrawCanvasLine(x2 ,y2 ,x1 ,y1);
        }

        if( size1 >= 1 || size2 >= 1){

            float distx = x1 - x2;
            float disty = y1 - y2;
            float a ,b;
            float r1 = size1 / 2.0f;
            float r2 = size2 / 2.0f;
            QPointF pt[4];

            a = (distx*distx + disty*disty);
            b = a - (r1-r2)*(r1-r2);
            if( b <= 0)
                goto LAST;
            b = sqrt( b);

            pt[0].setX(r1 * (distx * (r1-r2) + disty*b) / a + x2);
            pt[0].setY(r1 * (disty * (r1-r2) - distx*b) / a + y2);
            pt[1].setX(r1 * (distx * (r1-r2) - disty*b) / a + x2);
            pt[1].setY(r1 * (disty * (r1-r2) + distx*b) / a + y2);

            distx = x2 - x1;
            disty = y2 - y1;
            r1 = size2 / 2.0f;
            r2 = size1 / 2.0f;
            a = (distx*distx + disty*disty);
            b = a - (r1-r2)*(r1-r2);
            if( b <= 0)
                goto LAST;
            b = sqrt( b);
            pt[2].setX(r1 * (distx * (r1-r2) + disty*b) / a + x1);
            pt[2].setY(r1 * (disty * (r1-r2) - distx*b) / a + y1);
            pt[3].setX(r1 * (distx * (r1-r2) - disty*b) / a + x1);
            pt[3].setY(r1 * (disty * (r1-r2) + distx*b) / a + y1);



            float angle = atan2( -y ,x );
            if( size1 <= 1 ){//begin stroke
                float x = x2 + (x1 - x2) * (1 - size1) / (size2 - size1);
                float y = y2 + (y1 - y2) * (1 - size1) / (size2 - size1);
                pt[0].setX(x +  cos( angle + StaticData::PI/2) * .5);
                pt[1].setX(x +  cos( angle - StaticData::PI/2) * .5);
                pt[0].setY(y + -sin( angle + StaticData::PI/2) * .5);
                pt[1].setY(y + -sin( angle - StaticData::PI/2) * .5);
            }
            if( size2 <= 1 ){//end stroke
                float x = x1 + (x2 - x1) * (1 - size2) / (size1 - size2);
                float y = y1 + (y2 - y1) * (1 - size2) / (size1 - size2);
                pt[2].setX(x +  cos( angle - StaticData::PI/2) * .5);
                pt[3].setX(x +  cos( angle + StaticData::PI/2) * .5);
                pt[2].setY(y + -sin( angle - StaticData::PI/2) * .5);
                pt[3].setY(y + -sin( angle + StaticData::PI/2) * .5);
            }

            DrawCanvasPolygon( pt ,4);
        }
    }
    LAST:
    //Calculate the drawn rectangle.
    QRect rect;
    rect.setCoords(x2 ,y2 ,x1 ,y1);
    rect = rect.normalized();
    int size = ( size1 > size2 ) ? (size1)/2 + 1 : (size2+1) + 1;
    rect.setLeft(rect.left() - size);
    rect.setTop(rect.top() - size);
    rect.setRight(rect.right() + size +1);
    rect.setBottom(rect.bottom() + size +1);

    return rect;
}

int Canvas::DrawCanvasCircle(float x, float y, float r)
{
    float top ,bottom;
    float i ,wx;

    top = floor(y-r+.5) + .5;
    bottom = floor(y+r-.5) + .5;

    for( i=top ;i <= bottom ;i++){
        wx = sqrt( r*r - (y-i)*(y-i));
        DrawCanvasHLine(floor( x-wx+.5) ,floor( x+wx-.5) ,floor(i));
    }

    return 0;
}

int Canvas::DrawCanvasHLine(int x1, int x2, int y)
{
    unsigned int *p;
    unsigned int ptmp ,ptmp2;

    if( x1 > x2 || x2 < 0 || x1 >= bmp->width() || y < 0 || y >= bmp->height())
        return 0;
    x1 = x1 < 0 ? 0 : x1;
    x2 = x2 >= bmp->width() ? bmp->width()-1 : x2;

    int srcLine = (bmp->width() + 31) / 32;
    p = (unsigned int *)bmp->bits();
    //p += (bmp->height() - 1 - y) * srcLine;
    p += y * srcLine;

    ptmp = 0xffffff00u + (0xffu >> (x1&7)) << (x1 & 0x18);
    ptmp = ~ptmp;
    ptmp2 = 0x00ffffffu + (0xffu << 7 - (x2&7) << 24) >> (0x18 - (x2 & 0x18));
    ptmp2 = ~ptmp2;
    x1 >>= 5;
    x2 >>= 5;
    if( x1 == x2){
        ptmp |= ptmp2;
        if( color == 0)
            p[x1] &= ptmp;
        else
            p[x1] |= ~ptmp;
    }
    else{
        if( color == 0){
            p[x1] &= ptmp;
            for( x1++ ; x1 < x2 ;x1++)
                p[x1] = 0ul;
            p[x2] &= ptmp2;
        }
        else{
            p[x1] |= ~ptmp;
            for( x1++ ; x1 < x2 ;x1++)
                p[x1] = ~0ul;
            p[x2] |= ~ptmp2;
        }
    }
    return 0;
}

int Canvas::DrawCanvasPolygon( QPointF *pt ,size_t num)
{
    int top ,bottom;
    int left ,nextl ,right, nextr;
    int i;
    float x1 ,x2 ,y ,tmp;

    top = 0;
    bottom = 0;
    for( i=0 ;i<num ;i++){
        if( pt[i].y() < pt[top].y())
            top = i;
        if( pt[i].y() > pt[bottom].y() )
            bottom = i;
    }

    if( floor(pt[top].y()+.5) > floor(pt[bottom].y()-.5) ){
        return 0;
    } else if( floor(pt[top].y()+.5) == floor(pt[bottom].y()-.5) ){
        //水平に線を引いておしまい
        left = 0;
        right = 0;
        for( i=0 ;i<num ;i++){
            if( pt[i].x() < pt[left].x())
                left = i;
            if( pt[i].x() > pt[right].x())
                right = i;
        }
        x1 = pt[left].x();
        x2 = pt[right].x();
        DrawCanvasHLine(floor(x1+.5) ,floor(x2-.5) ,floor(pt[top].y()+.5));
    }
    else{
        i = top;
        while( pt[i].y() <= floor(pt[top].y()+.5))
            i = mod( i-1 ,num);
        left = mod( i+1 ,num);
        nextl = i;
        i = top;
        while( pt[i].y() <= floor(pt[top].y()+.5))
            i = mod( i+1 ,num);
        right = mod( i-1 ,num);
        nextr = i;

        for( y = floor(pt[top].y()+.5)+.5 ; y <= floor(pt[bottom].y()-.5)+.5 ;y++){
            if( pt[nextl].y() < y){
                do{
                    left = nextl;
                    nextl = mod( nextl-1 ,num);
                }while( pt[nextl].y() < y);
            }
            if( pt[nextr].y() < y){
                do{
                    right = nextr;
                    nextr = mod( nextr+1 ,num);
                }while( pt[nextr].y() < y);
            }
            x1 = (pt[nextl].x() - pt[ left].x()) * (y - pt[ left].y()) / (pt[nextl].y() - pt[ left].y()) + pt[ left].x();
            x2 = (pt[nextr].x() - pt[right].x()) * (y - pt[right].y()) / (pt[nextr].y() - pt[right].y()) + pt[right].x();
            if( x1 > x2){
                tmp = x1;
                x1 = x2;
                x2 = tmp;
            }
            DrawCanvasHLine(floor(x1+.5) ,floor(x2-.5) ,floor(y));
        }
    }

    return 0;
}

int Canvas::DrawCanvasLine(float x1 ,float y1 ,float x2 ,float y2)
{
    float lx ,ly;
    float d;
    float x ,y ,i;
    float tmp;
    float sign;

    lx = fabs(x2-x1);
    ly = fabs(y2-y1);
    if( lx <= ly){
        if( y1 > y2){
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
        x = x1;
        sign = x2-x1 >= 0 ? 1 : -1;
        //d = ly/2;
        d = sign==1 ? (x1-floor(x1))*ly : (1 - (x1-floor(x1)))*ly;
        for( i=y1 ;i<=y2 ;i++){
            bmp->setPixel(x, i, color);
            d += lx;
            if( d >= ly){
                d -= ly;
                x += sign;
            }
        }
    }
    else{
        if( x1 > x2){
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
        y = y1;
        sign = y2-y1 >= 0 ? 1 : -1;
        //d = lx/2;
        d = sign==1 ? (y1-floor(y1))*lx : (1 - (y1-floor(y1)))*lx;
        for( i=x1 ;i<=x2 ;i++){
            bmp->setPixel(i, y, color);
            d += ly;
            if( d >= lx){
                d -= lx;
                y += sign;
            }
        }
    }

    return 0;
}





QRect Canvas::drawPencil( const struct TabletData &lastPoint, const struct TabletData &currentPoint)
{
    static float rest = 0;

    qreal x2 = (lastPoint.pos.x() * StaticData::scale + .25f);
    qreal y2 = (lastPoint.pos.y() * StaticData::scale + .25f);
    qreal x1 = (currentPoint.pos.x() * StaticData::scale + .25f);
    qreal y1 = (currentPoint.pos.y() * StaticData::scale + .25f);

    QPointF fpoint1( x1 ,y1) ,fpoint2( x2 ,y2);
    QPoint point1( (int)x1 ,(int)y1) ,point2( (int)x2 ,(int)y2);
    qreal gamma;

    if( pen[penIndex].hard >= 50){
         gamma = ((qreal)pen[penIndex].hard - 50) / 50 * 3 + 1;
    }
    else{
        gamma = 1.0f / (((qreal)50 - pen[penIndex].hard) / 50 * 3 + 1);
    }


    qreal size1 = pow(lastPoint.pressure, gamma) * pen[penIndex].size;
    qreal size2 = pow(currentPoint.pressure, gamma) * pen[penIndex].size;

    if( lastPoint.pressure > 0.0f && currentPoint.pressure == 0.0f){
        rest = 0;
    }

    QRect rect;
   rect.setCoords(x2 ,y2 ,x1 ,y1);
   rect = rect.normalized();

    if( !(floor(x1) == floor(x2) && floor(y1) == floor(y2)) && (size1 >=MIN_SIZE || size2 >= MIN_SIZE) ){
        //float pos = 0.0f;
        float pos = -rest;
        float length;
        int distX ,distY;
        int posX ,posY;
        int tmpX ,tmpY;
        float d;
        QRect tmprt;

        QPoint p1(x2, y2) ,p2(x1, y1);
        length = distance( p1 ,p2);
        distX = p2.x() - p1.x();
        distY = p2.y() - p1.y();

        float angle = (-currentPoint.rotation + 90) / 180.0 * StaticData::PI;
        bool draw = false;
        int distanceStrokeDirection;
        while( pos < length){
            float tmpTilt;
            tmpTilt = currentPoint.tilt < 300.0f ? 0.0f : (currentPoint.tilt  - 300.0f) / 600.0f;

            d = size1 * (1.0f - pos / length) + size2 * pos / length;
            d *= (1.0f - tmpTilt) * 9.95f / 10.0f + .05f;
            if (d < 2) d = 2;

            float a = d / 2;
            float b = a * tmpTilt;

            posX = distX * pos / length + p1.x();
            posY = distY * pos / length + p1.y();

            distanceStrokeDirection = b * 2 / 10;
            if( distanceStrokeDirection < 1) distanceStrokeDirection = 1;

            if( pos + rest >= distanceStrokeDirection){
                float strength;

                strength = pow( lastPoint.pressure ,gamma);
                strength *= tmpTilt * 1.0f / 2.0f + 1.0f / 2.0f;
                DrawCanvasEllipse(posX ,posY ,a ,b ,angle, strength);
                pos += distanceStrokeDirection;
                rest = 0;
                draw = true;
                //描画した範囲を含む矩形を記憶
                tmprt.setLeft(posX - d/2);
                tmprt.setTop(posY - d/2);
                tmprt.setRight(posX + d/2 +1);
                tmprt.setBottom(posY + d/2 +1);
                rect |= tmprt;
            }
            else{
                pos += distanceStrokeDirection;
            }
        }

        if(draw){
            rest = length - ( pos - distanceStrokeDirection);
        }
        else{
            rest += length;
        }

    }

    return rect;
}

float Canvas::distance( const QPointF p1 ,const QPointF p2)
{
    return  sqrt( (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y()));
}

int Canvas::DrawCanvasEllipse(float x, float y, float a, float b, float angle, float strength)
{
    QPointF *pt;
    float *ptStrength;
    int numpt;
    float mx, f;
    int i;
    //FILE *fp;

    numpt = a <= 2 ? 4 : 4 * floor(a);
    pt = new QPointF[numpt];
    ptStrength = new float[numpt];

    mx = -a;
    for (i = 0; i < numpt / 4; i++){//最初の4分の1
        pt[i].setX(mx);
        pt[i].setY((f = sqrt((b * b) - (b * b) * (mx * mx) / (a * a))) > 0.0 ? f : 0.0);
        ptStrength[i] = (1.0f - (mx + a) / (2.0f * a) * (1.0f - (b / a) * (b / a))) * strength;//強さ
        if (ptStrength[i] > 1.0f){
            ptStrength[i] = 1.0f;
        }
        if (ptStrength[i] < 0.0f){
            ptStrength[i] = 0.0f;
        }
        mx += a / (numpt / 4);
    }
    pt[i].setX(0);
    pt[i].setY(b);
    ptStrength[i] = (1.0f - (mx + a) / (2.0f * a) * (1.0f - (b / a) * (b / a))) * strength;//強さ
    if (ptStrength[i] > 1.0f){
        ptStrength[i] = 1.0f;
    }
    if (ptStrength[i] < 0.0f){
        ptStrength[i] = 0.0f;
    }
    mx += a / (numpt / 4);
    for (i++; i <= numpt * 2 / 4; i++){//次の4分の1
        pt[i].setX(-pt[numpt / 2 - i].x());
        pt[i].setY(pt[numpt / 2 - i].y());
        ptStrength[i] = (1.0f - (mx + a) / (2.0f * a) * (1.0f - (b / a) * (b / a))) * strength;//強さ
        if (ptStrength[i] > 1.0f){
            ptStrength[i] = 1.0f;
        }
        if (ptStrength[i] < 0.0f){
            ptStrength[i] = 0.0f;
        }
        mx += a / (numpt / 4);
    }
    for (i = 1; i < numpt / 2; i++){//最後の4分の2
        pt[i + numpt / 2].setX(pt[numpt / 2 - i].x());
        pt[i + numpt / 2].setY(-pt[numpt / 2 - i].y());
        ptStrength[i + numpt / 2] = ptStrength[numpt / 2 - i];//強さ
    }

    //rotate and move
    //fp = fopen( "debug.txt" ,"a");
    for (i = 0; i < numpt; i++){
        float tmpx, tmpy;

        //fprintf( fp ,"%d x=%6f y=%6f ," ,i ,pt[i].x ,pt[i].y);
        tmpx = pt[i].x() * cos(-angle) + pt[i].y() * sin(-angle);
        tmpy = pt[i].x() * sin(-angle) + pt[i].y() *-cos(-angle);
        //tmpx = pt[i].x;
        //tmpy = pt[i].y;
        tmpx = tmpx + x;
        tmpy = tmpy + y;
        pt[i].setX(tmpx);
        pt[i].setY(tmpy);
    }
    //fprintf( fp ,"\n");
    //fclose( fp);

    DrawCanvasPolygonWithStrength(pt, numpt, ptStrength);
    //DrawCanvasPolygon(pt, numpt);

    delete[] pt;
    delete[] ptStrength;

    return 0;
}

int Canvas::DrawCanvasPolygonWithStrength(QPointF *pt, size_t num, float *ptStrength)
{
    int top, bottom;
    int left, nextl, right, nextr;
    int i;
    float x1, x2, y, tmp;
    float strengthR, strengthL;

    top = 0;
    bottom = 0;
    for (i = 0; i<num; i++){
        if (pt[i].y() < pt[top].y())
            top = i;
        if (pt[i].y() > pt[bottom].y())
            bottom = i;
    }

    if (floor(pt[top].y() + .5) > floor(pt[bottom].y() - .5)){
        return 0;
    }
    else if (floor(pt[top].y() + .5) == floor(pt[bottom].y() - .5)){
        //水平に線を引いておしまい
        left = 0;
        right = 0;
        for (i = 0; i<num; i++){
            if (pt[i].x() < pt[left].x()){
                left = i;
            }
            if (pt[i].x() > pt[right].x()){
                right = i;
            }
        }
        x1 = pt[left].x();
        x2 = pt[right].x();
        DrawCanvasHLineWithStrength(floor(x1 + .5), floor(x2 - .5), floor(pt[top].y() + .5), ptStrength[left], ptStrength[right]);
    }
    else{
        i = top;
        while (pt[i].y() <= floor(pt[top].y() + .5))
            i = mod(i - 1, num);
        left = mod(i + 1, num);
        nextl = i;
        i = top;
        while (pt[i].y() <= floor(pt[top].y() + .5))
            i = mod(i + 1, num);
        right = mod(i - 1, num);
        nextr = i;

        for (y = floor(pt[top].y() + .5) + .5; y <= floor(pt[bottom].y() - .5) + .5; y++){
            if (pt[nextl].y() < y){
                do{
                    left = nextl;
                    nextl = mod(nextl - 1, num);
                } while (pt[nextl].y() < y);
            }
            if (pt[nextr].y() < y){
                do{
                    right = nextr;
                    nextr = mod(nextr + 1, num);
                } while (pt[nextr].y() < y);
            }
            x1 = (pt[nextl].x() - pt[left].x()) * (y - pt[left].y()) / (pt[nextl].y() - pt[left].y()) + pt[left].x();
            x2 = (pt[nextr].x() - pt[right].x()) * (y - pt[right].y()) / (pt[nextr].y() - pt[right].y()) + pt[right].x();
            //strengthL = (ptStrength[nextl] - ptStrength[left]) * (y - pt[left].y) / (pt[nextl].y - pt[left].y) + ptStrength[left];
            //strengthR = (ptStrength[nextr] - ptStrength[right]) * (y - pt[right].y) / (pt[nextr].y - pt[right].y) + ptStrength[right];
            strengthL = ptStrength[left];
            strengthR = ptStrength[right];
            if (x1 > x2){
                tmp = x1;
                x1 = x2;
                x2 = tmp;
            }
            DrawCanvasHLineWithStrength(floor(x1 + .5), floor(x2 - .5), floor(y), strengthL, strengthR);
        }
    }

    return 0;
}


int Canvas::DrawCanvasHLineWithStrength(int x1, int x2, int y, float strengthL, float strengthR)
{
    unsigned int *p;
    unsigned int ptmp, ptmp2;
    int x, i;
    int strInt;
    int diffStrInt = 0;
    int rand_threashold;
    int diff_threashold = 0;
    unsigned int tmp;
    //unsigned int *matLine = NULL;
    unsigned int matPixel;

    if (x1 > x2 || x2 < 0 || x1 >= bmp->width() || y < 0 || y >= bmp->height())
        return 0;
    x1 = x1 < 0 ? 0 : x1;
    x2 = x2 >= bmp->width() ? bmp->width() - 1 : x2;


    strInt = (double)strengthL / 2.0 * 255.0 * 65536.0;
    rand_threashold = UINT_MAX * (double)strengthL / 2.0 / 4.0;


    //matLine = &mat->pixel[(y % 512) * (512 / 4)];

    p = (unsigned int *)bmp->bits();
    p += y * (bmp->width() + 31 >> 5);

    ptmp = 0xffffff00u + (0xffu >> (x1 & 7)) << (x1 & 0x18);
    ptmp = ~ptmp;
    ptmp2 = 0x00ffffffu + (0xffu << 7 - (x2 & 7) << 24) >> (0x18 - (x2 & 0x18));
    ptmp2 = ~ptmp2;
    diff_threashold = ((UINT_MAX * (double)strengthR / 2.0 / 4.0) - rand_threashold) / (x2 - x1 + 1);
    diffStrInt = ((double)strengthR / 2.0 * 255.0 * 65536.0 - strInt) / (x2 - x1 + 1);
    strInt -= diffStrInt * (x1 - (x1 & ~31));

    //x1 >>= 5;
    //x2 >>= 5;
    if ((x1 >> 5) == (x2 >> 5)){
        unsigned int DWPixel = 0;
        int xIndex = x1 & ~31;

        for (int i = 0; i < 4; i++){
            unsigned int rnd;
            unsigned char byte = 0;

            matPixel = 0xffffffff;
            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            matPixel = 0xffffffff;
            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            rnd = rand_xor128();
            byte <<= 1;
            byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
            strInt += diffStrInt;

            xIndex += 8;

            DWPixel >>= 8;
            DWPixel += (unsigned int)byte << 24;
        }

        //DWPixel = 0x0;


        ptmp |= ptmp2;
        if (color == 0)
            p[x1 >> 5] &= (ptmp | DWPixel);
        else
            p[x1 >> 5] |= ~(ptmp | DWPixel);
    }
    else{
        if (color == 0){//描画
            unsigned int rnd;
            unsigned int DWPixel = 0;
            int xIndex = x1 & ~31;
            //begin debug
            //DebugDlgInt(matPixel);
            //end debug

            for (int i = 0; i < 4; i++){
                unsigned int rnd;
                unsigned char byte = 0;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                matPixel = 0xffffffff;
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                xIndex += 8;

                DWPixel >>= 8;
                DWPixel += (unsigned int)byte << 24;
            }

            //DWPixel = 0x0;

            p[x1 >> 5] &= (ptmp | DWPixel);
            x1 = (x1 + 31) & ~31;

            for (; (x1 >> 5) < (x2 >> 5);){

                for (int i = 0; i < 4; i++){
                    unsigned int rnd;
                    unsigned char byte = 0;

                    rnd = rand_xor128();
                    matPixel = 0xffffffff;
                    byte <<= 1;
                    byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    matPixel = 0xffffffff;
                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    xIndex += 8;

                    DWPixel >>= 8;
                    DWPixel += (unsigned int)byte << 24;
                }

                //DWPixel = 0x0;

                p[x1 >> 5] &= (DWPixel);
                x1 += 32;
            }

            for (int i = 0; i < 4; i++){
                unsigned int rnd;
                unsigned char byte = 0;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                xIndex += 8;

                DWPixel >>= 8;
                DWPixel += (unsigned int)byte << 24;
            }

            //DWPixel = 0x0;

            p[x2 >> 5] &= (ptmp2 | DWPixel);
        }
        else{//消去
            unsigned int DWPixel = 0;
            int xIndex = x1 & ~31;
            //begin debug
            //DebugDlgInt(matPixel);
            //end debug

            for (int i = 0; i < 4; i++){
                unsigned int rnd;
                unsigned char byte = 0;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                xIndex += 8;

                DWPixel >>= 8;
                DWPixel += (unsigned int)byte << 24;
            }

            //DWPixel = 0x0;

            p[x1 >> 5] |= ~(ptmp | DWPixel);
            x1 = (x1 + 31) & ~31;

            for (; (x1 >> 5) < (x2 >> 5);){

                for (int i = 0; i < 4; i++){
                    unsigned int rnd;
                    unsigned char byte = 0;

                    matPixel = 0xffffffff;
                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    matPixel = 0xffffffff;
                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    rnd = rand_xor128();
                    byte <<= 1;
                    byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                    strInt += diffStrInt;

                    xIndex += 8;

                    DWPixel >>= 8;
                    DWPixel += (unsigned int)byte << 24;
                }

                //DWPixel = 0x0;

                p[x1 >> 5] |= ~DWPixel;
                x1 += 32;
            }

            for (int i = 0; i < 4; i++){
                unsigned int rnd;
                unsigned char byte = 0;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                matPixel = 0xffffffff;
                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 8 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 16 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                rnd = rand_xor128();
                byte <<= 1;
                byte += (int)(255 - (matPixel >> 24 & 0xff)) <= (strInt >> 16) && rnd >> 2 <= rand_threashold ? 0 : 1;
                strInt += diffStrInt;

                xIndex += 8;

                DWPixel >>= 8;
                DWPixel += (unsigned int)byte << 24;
            }

            //DWPixel = 0x0;

            p[x2 >> 5] |= ~(ptmp2 | DWPixel);
        }
    }



    return 0;
}


unsigned int Canvas::rand_xor128()
{
    static unsigned int x=123456789,y=362436069,z=521288629,w=88675123;
    unsigned int t;
    t=(x^(x<<11));x=y;y=z;z=w;
    return ( w=(w^(w>>19))^(t^(t>>8)) );
}







/*
QRect Canvas::drawEraser( int x1 ,int y1 ,int width1 ,int x2 ,int y2 ,int width2)
{
    QPainter p( bmp);
    QPointF fpoint1( x1+.25 ,y1+.25) ,fpoint2( x2+.25 ,y2+.25);
    QPoint point1( x1 ,y1) ,point2( x2 ,y2);
    //QRect rect;
    qreal gamma;

    if( pen[penIndex].hard >= 50){
         gamma = ((qreal)pen[penIndex].hard - 50) / 50 * 3 + 1;
    }
    else{
        gamma = 1.0f / (((qreal)50 - pen[penIndex].hard) / 50 * 3 + 1);
    }

    qreal size = pow( (qreal)width2 / 255.0 ,gamma) * pen[penIndex].size;

    BmpModified = true;
    p.setPen(QPen( Qt::white, size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine( fpoint1 ,fpoint2);
    //rect = drawPen( x1, y1 ,width1 ,x2 ,y2 ,width2 ,false);
    //return reduce( rect);
    //return reduce( QRect( point1 ,point2).normalized().adjusted( -width2/2-1 ,-width2/2-1 ,width2/2+1 ,width2/2+1));

    QRect rect = QRect( point1 ,point2).normalized().adjusted( -size/2-1 ,-size/2-1 ,size/2+1 ,size/2+1);
    if( rect.left() < 0) rect.setLeft( 0);
    if( rect.top() < 0) rect.setTop( 0);
    if( rect.right() > bmp->width()) rect.setRight( bmp->width());
    if( rect.bottom() > bmp->height()) rect.setBottom( bmp->height());
    rect.setLeft( rect.left() & ~31);
    rect.setTop( rect.top() & ~7);
    rect.setRight( rect.right() + 31 & ~31);
    rect.setBottom( rect.bottom() + 7 & ~7);

    return rect;
}*/

UpdateObject *Canvas::reduce( QRect rect)
{
    QImage *out;
    QRect *rect2;
    //UpdateObject *uo;
    int outWidth ,outHeight;
    unsigned int *src ,*dst;
    int srcLine ,dstLine;
    int rectLeft ,rectTop ,rectRight ,rectBottom;

    if( rect.left() < 0) rect.setLeft( 0);
    if( rect.top() < 0) rect.setTop( 0);
    if( rect.right() > bmp->width()) rect.setRight( bmp->width());
    if( rect.bottom() > bmp->height()) rect.setBottom( bmp->height());

    rect.setLeft( rect.left() & ~31);
    rect.setTop( rect.top() & ~7);
    rect.setRight( rect.right() + 31 & ~31);
    rect.setBottom( rect.bottom() + 7 & ~7);

    outWidth = (rect.right() - rect.left()) / StaticData::scale;
    outHeight = (rect.bottom() - rect.top()) / StaticData::scale;
    out = new QImage( outWidth ,outHeight ,QImage::Format_Indexed8);

    for( int i=0 ; i < numColors ; ++i){
        int c;
        if(drawingMode == EPFrameBuffer::WaveformMode::Mono){
            c = palleteMono[i];
        }
        else{
            c = palleteGray[i];
        }
        out->setColor( i, qRgb( c, c, c));
    }

    src = (unsigned int *)bmp->bits();
    dst = (unsigned int *)out->bits();
    srcLine = (bmp->width() + 31) / 32;
    dstLine = (out->width() + 3) / 4;
    rectLeft = rect.left();
    rectTop = rect.top();
    rectRight = rect.right();
    rectBottom = rect.bottom();
    ////QMessageBox::information( 0 ,tr("Debug") ,tr("before reduce"));

    for( int y = rectTop ; y + (StaticData::scale - 1) < rectBottom ; y += StaticData::scale){
        for( int x = rectLeft ; x+31 < rectRight ; x += 32){
            unsigned int sum1 = 0;
            unsigned int sum2 = 0;
            for( int i=0 ; i < StaticData::scale ; ++i){
                unsigned int tmp;
                tmp = src[x/32 + srcLine*(y+i)];
                tmp = (tmp & 0x55555555) + (tmp>>1 & 0x55555555);
                tmp = (tmp & 0x33333333) + (tmp>>2 & 0x33333333);

                sum1 += (tmp<<16 & 0x0f000000) + (tmp<<4 & 0x0f0000) + (tmp<<8 & 0x0f00) + (tmp>>4 & 0x0f);
                sum2 += (tmp & 0x0f000000) + (tmp>>12 & 0x0f0000) + (tmp>>8 & 0x0f00) + (tmp>>20 & 0x0f);
            }
            dst[ (x - rectLeft) / StaticData::scale / 4 + 0 + dstLine * (y - rectTop) / StaticData::scale] = sum1;
            dst[ (x - rectLeft) / StaticData::scale / 4 + 1 + dstLine * (y - rectTop) / StaticData::scale] = sum2;
        }
    }

    //bmp->fill( 0u);

    rect2 = new QRect();
    rect2->setLeft( rect.left() / StaticData::scale);
    rect2->setTop( rect.top() / StaticData::scale);
    rect2->setRight( rect.right() / StaticData::scale);
    rect2->setBottom( rect.bottom() / StaticData::scale);

    ////QMessageBox::information( 0 ,tr("Debug") ,tr("reduced"));

    return new UpdateObject( out ,rect2);
}

void Canvas::monoImageCopy( QImage &dstImage, QRect dstRect ,QImage &srcImage, QRect srcRect)
{
    unsigned int *src ,*dst;
    int srcLine ,dstLine;
    int rectLeft ,rectTop ,rectRight ,rectBottom;
    int dstLeft ,dstTop;

    if( srcRect.left() < 0) srcRect.setLeft( 0);
    if( srcRect.top() < 0) srcRect.setTop( 0);
    if( srcRect.right() > srcImage.width()) srcRect.setRight( srcImage.width());
    if( srcRect.bottom() > srcImage.height()) srcRect.setBottom( srcImage.height());

    srcRect.setLeft( srcRect.left() & ~31);
    srcRect.setTop( srcRect.top() & ~7);
    srcRect.setRight( srcRect.right() + 31 & ~31);
    srcRect.setBottom( srcRect.bottom() + 7 & ~7);

    if( dstRect.left() < 0) dstRect.setLeft( 0);
    if( dstRect.top() < 0) dstRect.setTop( 0);
    if( dstRect.right() > srcImage.width()) dstRect.setRight( srcImage.width());
    if( dstRect.bottom() > srcImage.height()) dstRect.setBottom( srcImage.height());

    dstRect.setLeft( dstRect.left() & ~31);
    dstRect.setTop( dstRect.top() & ~7);
    dstRect.setRight( dstRect.right() + 31 & ~31);
    dstRect.setBottom( dstRect.bottom() + 7 & ~7);

    //src = (unsigned int *)srcImage.bits();
    //dst = (unsigned int *)dstImage.bits();
    srcLine = srcImage.width() / 32;
    dstLine = dstImage.width() / 32;
    rectLeft = srcRect.left();
    rectTop = srcRect.top();
    rectRight = srcRect.right();
    rectBottom = srcRect.bottom();
    dstLeft = dstRect.left();
    dstTop = dstRect.top();
    ////QMessageBox::information( 0 ,tr("Debug") ,tr("before reduce"));

    int dstY = dstTop;
    for( int y = rectTop ; y < rectBottom ; ++y){
        src = (unsigned int *)srcImage.scanLine( y);
        dst = (unsigned int *)dstImage.scanLine( dstY++);
        src += rectLeft / 32;
        dst += dstLeft / 32;
        for( int x = rectLeft ; x+31 < rectRight ; x+=32){
            *dst++ = *src++;
        }
    }

}

QImage *Canvas::createMonoImage( QImage &srcImage, QRect rect)
{
    unsigned int *src ,*dst;
    //int srcLine ,dstLine;
    int rectLeft ,rectTop ,rectRight ,rectBottom;

    if( rect.left() < 0) rect.setLeft( 0);
    if( rect.top() < 0) rect.setTop( 0);
    if( rect.right() > srcImage.width()) rect.setRight( srcImage.width());
    if( rect.bottom() > srcImage.height()) rect.setBottom( srcImage.height());

    rect.setLeft( rect.left() & ~31);
    rect.setTop( rect.top() & ~7);
    rect.setRight( rect.right() + 31 & ~31);
    rect.setBottom( rect.bottom() + 7 & ~7);

    QImage *dstImage = 0;
    dstImage = new QImage( rect.width() ,rect.height() ,QImage::Format_Mono);

    //src = (unsigned int *)srcImage.bits();
    //dst = (unsigned int *)dstImage.bits();
    //srcLine = srcImage.width() / 32;
    //dstLine = dstImage->width() / 32;
    rectLeft = rect.left();
    rectTop = rect.top();
    rectRight = rect.right();
    rectBottom = rect.bottom();
    ////QMessageBox::information( 0 ,tr("Debug") ,tr("before reduce"));

    for( int y = rectTop ; y < rectBottom ; ++y){
        src = (unsigned int *)srcImage.scanLine( y);
        dst = (unsigned int *)dstImage->scanLine( y - rectTop);
        src += rectLeft / 32;
        for( int x = rectLeft ; x+31 < rectRight ; x+=32){
            *dst++ = *src++;
        }
    }

    return dstImage;
}

void Canvas::setDrawingMode(EPFrameBuffer::WaveformMode drawingMode)
{
    this->drawingMode = drawingMode;

    //if(drawingMode == EPFrameBuffer::WaveformMode::Mono){
    //    penIndex = 0;
    //}
    //else{
    //    penIndex = 1;
    //}

    //penIndex = (penIndex + 1) % 2;
}

void Canvas::togglePen()
{
    penIndex = (penIndex + 1) % 2;
}

int Canvas::mod(int x, int y)//剰余が負の数になってしまう事への対策
{
    return x%y >= 0 ? x%y : x%y + y;
}




UpdateObject::UpdateObject( QImage *bmp, QRect *rect)
{
    m_bmp = bmp;
    m_rect = rect;
}

UpdateObject::~UpdateObject()
{
    delete m_bmp;
    delete m_rect;
}
