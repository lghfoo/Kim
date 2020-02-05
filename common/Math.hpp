#pragma once
#include<QPointF>
#include<QtMath>
namespace Kim {
    static qreal KSquaredDistance(const QPointF& Point){
        return Point.x()*Point.x() + Point.y()*Point.y();
    }
    static qreal KDistance(const QPointF& Point){
        return qSqrt(KSquaredDistance(Point));
    }
    static qreal KDistance(const QPointF& P0, const QPointF& P1){
        return KDistance(P1 - P0);
    }
    static bool KNormalize(QPointF& Point){
        auto Distance = KDistance(Point);
        if(Distance > 0){
            Point /= Distance;
            return true;
        }
        return false;
    }

    static QPointF KToNormalized(const QPointF& Point){
        QPointF Ret = Point;
        KNormalize(Ret);
        return Ret;
    }
}
