#ifndef STATICDATA_H
#define STATICDATA_H

#include <QObject>
#include <QString>

struct StaticData
{
    const static QString appName;
    const static QString imagePath;
    const static int screenWidth = 1404;
    const static int screenHeight = 1872;
    const static int scale = 4;
    const static int penNum = 3;
    constexpr const static qreal PI = 3.14159265358979f;
};

#endif // STATICDATA_H
