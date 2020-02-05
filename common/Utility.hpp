#pragma once
#include <QString>
#include <QDateTime>
#include <QDebug>
#include <QWidget>
#include<QPaintEvent>
#include <QPainter>
#include<QFontDialog>
#include <QColorDialog>
#include <QTextLayout>
#include <QLinkedList>
#include<QGraphicsItem>
#include"Math.hpp"
namespace Kim {
    enum ViewType{
        UserType = QGraphicsItem::UserType,
        ConnectionType = UserType + 1,
        ControlPointType = UserType + 2,
        TextItemType = UserType + 3,
        ImageItemType = UserType + 4
                 };
    static const QString IdentityTimeFormat = "yyyy_MM_dd_hh_mm_ss_zzz";
    static const QString NormalTimeFormat = "yyyy/MM/dd hh:mm:ss";
    enum SelectionType{
        All, None, Reverse
    };

    static bool InRange(qreal Value, qreal From, qreal To){
        return (From < Value && Value < To)
                ||(To < Value && Value < From);
    }

    static QPointF Projection(const QPointF& FromVec, const QPointF& ToVec){
        qreal DotProduct =  QPointF::dotProduct(ToVec, ToVec);
        if(qAbs(DotProduct) < 0.001)return {0, 0};
        return (QPointF::dotProduct(FromVec, ToVec) / DotProduct) * ToVec;
    }

    static void ToClosestPoint(QPointF& Point,
                        const QPointF& From, const QPointF& To){
        auto PointVec = Point - From;
        auto ToVec = To - From;
        auto Proj = Projection(PointVec, ToVec);
        Point = From + Proj;
    }

    static qreal PointDistanceToLine(const QPointF& Point, const QLineF& Line, bool& IsInBound){
        const auto& P0 = Line.p1();
        const auto& P1 = Line.p2();
        const auto& V0 = Point - P0;
        const auto& V1 = P1 - P0;
        const auto& Proj = Projection(V0, V1);
        IsInBound = InRange(Proj.x(), 0, V1.x())
                && InRange(Proj.y(), 0, V1.y());
        if(IsInBound){
            return KDistance(Proj - V0);
        }
        else{
            auto Dis0 = KDistance(Point - P0);
            auto Dis1 = KDistance(Point - P1);
            return qMin(Dis0, Dis1);
        }
    }

    static QPointF QuadValue(const QPointF& From, const QPointF& Ctrl, const QPointF& To, qreal T){
        return (1-T)*(1-T)*From + 2 * (1-T) * T * Ctrl + T * T * To;
    }

    static QPointF CubicValue(const QPointF& From, const QPointF& C1, const QPointF& C2, const QPointF& To, qreal T){
        return (1-T)*(1-T)*(1-T)*From + 3 * (1-T) * (1-T) * T * C1 + 3 * (1-T) * T * T * C2 + T * T * T * To;
    }

    static QPointF CubicDerived(const QPointF& From, const QPointF& C1, const QPointF& C2, const QPointF& To, qreal T){
        return -3*(1-T)*(1-T)*From + 3*(-2*(1-T)*T+(1-T)*(1-T))* C1 + 3*(-T*T+(1-T)*2*T)*C2 + 3*T*T*To;
    }

    static QPointF QuadDerived(const QPointF& From, const QPointF& C1, const QPointF& To, qreal T){
        return -2*(1-T)*From + 2*(-2*T+1)*C1 + 2*T*To;
    }

    static qreal DegreeBetween(const QPointF& V0, const QPointF& V1){
        auto V2 = Projection(V0, V1);
        auto V2Dis = KDistance(V2);
        if(V2Dis < 0.001)return 0;
        auto V0Dis = KDistance(V0);
        auto Dot = QPointF::dotProduct(V0, V2);
        auto Cos = Dot / (V0Dis * V2Dis);
        auto Rad = qAcos(Cos);
        return Rad / M_PI * 180;
    }

    static void CreateQuadLines(QVector<QLineF>& Lines, const QPointF& From, const QPointF& Ctrl, const QPointF& To){
       int Sample = 16;
       qreal Delta = 1.0 / Sample;
        for(int i = 0; i < Sample; i++){
            qreal CurrentT = Delta * i;
            qreal NextT = Delta* (i+1);
            auto P0 = QuadValue(From, Ctrl, To, CurrentT);
            auto P1 = QuadValue(From, Ctrl, To, NextT);
            Lines.append({P0, P1});
        }
    }

    static void CreateQuadPoints(QLinkedList<QPointF>& Points, QLinkedList<QPointF>::iterator Iter,
                                  const QPointF& From, const QPointF& C1, const QPointF& To, qreal T,
                                  qreal FromT, qreal ToT){
        const qreal Threshold = 1;
        auto Point = QuadValue(From, C1, To, T);
        Iter = Points.insert(Iter, Point);
        auto Derived = QuadDerived(From, C1, To, T);
        auto P0 = *(Iter-1);
        auto V0 = P0 - Point;
        auto Deg0 = DegreeBetween(V0, Derived);
        if(Deg0 > Threshold){
//            qDebug()<<"left"<<Deg0<<Threshold<<(T+FromT)/2<<FromT<<T<<ToT;
//            qDebug()<<V0<<Derived;
            CreateQuadPoints(Points, Iter, From, C1, To, (T+FromT)/2, FromT, T);
        }
        auto P1 = *(Iter+1);
        auto V1 = P1 - Point;
        auto Deg1 = DegreeBetween(V1, Derived);
        if(Deg1 > Threshold){
//            qDebug()<<"right"<<Deg1<<Threshold<<(T+ToT)/2<<FromT<<T<<ToT;
            CreateQuadPoints(Points, Iter+1, From, C1, To, (T+ToT)/2, T, ToT);
        }
    }

    static void CreateQuadPoints(QVector<QPointF>& Points, const QPointF& From, const QPointF& Ctrl, const QPointF& To){
//       int Sample = 16;
//       qreal Delta = 1.0 / Sample;
//        for(int i = 0; i <= Sample; i++){
//            qreal CurrentT = Delta * i;
//            auto P0 = QuadValue(From, Ctrl, To, CurrentT);
//            Points.append(P0);
//        }
        QLinkedList<QPointF>TmpPoints{From, To};
        CreateQuadPoints(TmpPoints, TmpPoints.begin() + 1, From, Ctrl, To, 0.5, 0, 1);
        for(const auto& P : TmpPoints){
            Points.append(P);
        }
    }

    static void CreateCubicLines(QVector<QLineF>& Lines, const QPointF& From, const QPointF& C1, const QPointF& C2, const QPointF& To){
       int Sample = 16;
       qreal Delta = 1.0 / Sample;
        for(int i = 0; i < Sample; i++){
            qreal CurrentT = Delta * i;
            qreal NextT = Delta* (i+1);
            auto P0 = CubicValue(From, C1, C2, To, CurrentT);
            auto P1 = CubicValue(From, C1, C2, To, NextT);
            Lines.append({P0, P1});
        }
    }

    static void CreateCubicPoints(QLinkedList<QPointF>& Points, QLinkedList<QPointF>::iterator Iter,
                                  const QPointF& From, const QPointF& C1, const QPointF& C2, const QPointF& To, qreal T,
                                  qreal FromT, qreal ToT){
        const qreal Threshold = 1;
        auto Point = CubicValue(From, C1, C2, To, T);
        Iter = Points.insert(Iter, Point);
        auto Derived = CubicDerived(From, C1, C2, To, T);
        auto P0 = *(Iter-1);
        auto V0 = P0 - Point;
        auto Deg0 = DegreeBetween(V0, Derived);
        if(Deg0 > Threshold){
//            qDebug()<<"left"<<Deg0<<Threshold<<(T+FromT)/2<<FromT<<T<<ToT;
//            qDebug()<<V0<<Derived;
            CreateCubicPoints(Points, Iter, From, C1, C2, To, (T+FromT)/2, FromT, T);
        }
        auto P1 = *(Iter+1);
        auto V1 = P1 - Point;
        auto Deg1 = DegreeBetween(V1, Derived);
        if(Deg1 > Threshold){
//            qDebug()<<"right"<<Deg1<<Threshold<<(T+ToT)/2<<FromT<<T<<ToT;
            CreateCubicPoints(Points, Iter+1, From, C1, C2, To, (T+ToT)/2, T, ToT);
        }
    }

    static void CreateCubicPoints(QVector<QPointF>& Points, const QPointF& From, const QPointF& C1, const QPointF& C2, const QPointF& To){
//       int Sample = 16;
//       qreal Delta = 1.0 / Sample;
//        for(int i = 0; i <= Sample; i++){
//            qreal CurrentT = Delta * i;
//            auto P0 = CubicValue(From, C1, C2, To, CurrentT);
//            Points.append(P0);
//        }
        QLinkedList<QPointF>TmpPoints{From, To};
        CreateCubicPoints(TmpPoints, TmpPoints.begin() + 1, From, C1, C2, To, 0.5, 0, 1);
        for(const auto& P : TmpPoints){
            Points.append(P);
        }
    }

    static void ExtendRect(QRectF& Rect, const QPointF& P){
        auto X = Rect.x();
        auto Y = Rect.y();
        auto W = Rect.width();
        auto H = Rect.height();
        if(P.x() < X){
            auto DX = X - P.x();
            X = P.x();
            W += DX;
        }
        else if(P.x() > X + W){
            W += (P.x() - (X+W));
        }
        if(P.y() < Y){
            auto DY = Y - P.y();
            Y = P.y();
            H += DY;
        }
        else if(P.y() > Y + H){
            H += (P.y() - (Y+H));
        }
        Rect.setRect(X, Y, W, H);
    }

    static void ExtendRect(QRectF& Rect, const QRectF& R){
        if(Rect.isNull()){
            Rect = R;
        }
        else{
            ExtendRect(Rect, R.topLeft());
            ExtendRect(Rect, R.topRight());
            ExtendRect(Rect, R.bottomLeft());
            ExtendRect(Rect, R.bottomRight());
        }
    }

    static void ExtendRect(QRectF& Rect, const QLineF& L){
        const auto& From = L.p1();
        const auto& To = L.p2();
        qreal X = std::min(From.x(), To.x());
        qreal Y = std::min(From.y(), To.y());
        qreal W = std::abs(To.x() - From.x());
        qreal H = std::abs(To.y() - From.y());
        ExtendRect(Rect, QRectF(X, Y, W, H));
    }

    static qint64 CreateID(){
       auto Now = QDateTime::currentDateTime().toString(IdentityTimeFormat).split('_').join("");
       return Now.toLongLong();
    }

    static int GetFarestColor(int C){
        if(C < 128)return 255;
        else return 0;
    }

    static QColor GetFarestColor(const QColor& C){
        return {
            GetFarestColor(C.red()),
            GetFarestColor(C.green()),
            GetFarestColor(C.blue())
        };
    }

    static QRectF SetupTextLayout(QTextLayout *layout)
    {
        layout->setCacheEnabled(true);
        layout->beginLayout();
        while (layout->createLine().isValid());
        layout->endLayout();
        qreal maxWidth = 0;
        qreal y = 0;
        for (int i = 0; i < layout->lineCount(); ++i) {
            QTextLine line = layout->lineAt(i);
            maxWidth = qMax(maxWidth, line.naturalTextWidth());
            line.setPosition(QPointF(0, y));
            y += line.height();
        }
        return QRectF(0, 0, maxWidth, y);
    }

    static QRectF GetTextSize(const QString& Text, const QFont& Font) {
        QRectF br;
        if (Text.isEmpty()) {
            br = QRectF();
        } else {
            QString tmp = Text;
            tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
            QTextLayout layout(tmp, Font);
            br = SetupTextLayout(&layout);
        }
        return br;
    }

    static QRectF PaddingIn(const QRectF& Rect, qreal Padding){
        qreal X = Rect.x() + Padding;
        qreal Y = Rect.y() + Padding;
        qreal W = Rect.width() - 2 * Padding;
        qreal H = Rect.height() - 2 * Padding;
        return {X, Y, W, H};
    }

    static QRectF PaddingOut(const QRectF& Rect, qreal Padding){
        qreal X = Rect.x() - Padding;
        qreal Y = Rect.y() - Padding;
        qreal W = Rect.width() + 2 * Padding;
        qreal H = Rect.height() + 2 * Padding;
        return {X, Y, W, H};
    }

    //////////////////////////////// GUI Utility ////////////////////////////////
    class KColorPicker : public QWidget{
        Q_OBJECT
    signals:
        void ColorChangedSignal(const QColor& Color);
    private:
        QColor CurrentColor = Qt::white;
        QString Label = "Label";
    protected:
        virtual void paintEvent(QPaintEvent* Event)override{
            QPainter Painter(this);
            Painter.setRenderHint(QPainter::Antialiasing);
            QPen Pen;
            Pen.setColor(Qt::black);
            Pen.setWidth(2);
            Pen.setCapStyle(Qt::PenCapStyle::RoundCap);

            qreal Padding = 5;
            auto Rect = Event->rect();
            auto W = Rect.width() - 2 * Padding;
            auto H = Rect.height() - 2 * Padding;
            auto X = Rect.x() + Padding;
            auto Y = Rect.y() + Padding;
            auto TargetRect = QRectF(X, Y, W, H);

            QPainterPath Path;
            Path.addRoundedRect(TargetRect, 5, 5);
            Painter.fillPath(Path, CurrentColor);
            Painter.setPen(Pen);
            Painter.drawPath(Path);

//            Painter.setCompositionMode(QPainter::CompositionMode_Difference);
            Painter.setFont(QFont("Times", 12));
            Pen.setColor(GetFarestColor(CurrentColor));
            Painter.setPen(Pen);
            Painter.drawText(Event->rect(), Qt::AlignCenter, Label);
//            Painter.fillRect(TargetRect, CurrentColor);
//            Painter.drawRect(TargetRect);
        }
        virtual void mousePressEvent(QMouseEvent* Event)override{
            QColorDialog ColorDialog;
            auto Color = QColorDialog::getColor(CurrentColor, this, "Choose Color");
            if(Color.isValid()){
                this->SetColor(Color);
            }
//            return QWidget::mousePressEvent(Event);
        }
    public:
        KColorPicker(const QString& Text = "", const QColor& Color = Qt::black)
            :Label(Text), CurrentColor(Color){
            this->setMaximumHeight(40);
            this->setMinimumHeight(40);
//            this->setMaximumSize(80, 40);
//            this->setMinimumSize(80, 40);
        }

        QColor GetColor(){
            return CurrentColor;
        }

        void SetColor(const QColor& Color){
            if(this->CurrentColor != Color){
                emit ColorChangedSignal(Color);
                this->CurrentColor = Color;
                this->update();
            }
        }

        void SetText(const QString& Text){
            this->Label = Text;
        }
    };

    class KFontPicker : public QWidget{
        Q_OBJECT
    signals:
        void FontChangedSignal(const QFont& Font);
    private:
        QFont CurrentFont = QFont("Times", 12);
        QString GetText(){
            return CurrentFont.family() + QString(",") + QString::number(CurrentFont.pointSize());
        }
        QRectF GetTextRect(){
            auto Text = GetText();
//            auto Rect = GetTextSize(Text, CurrentFont);
            QFontMetrics Metrics(CurrentFont);
            return Metrics.boundingRect(GetText());
        }
        void UpdateSize(){
            auto Rect = GetTextRect();
            qreal Padding = 5;
            this->setMinimumHeight(Rect.height() + Padding * 2);
            this->setMaximumHeight(Rect.height() + Padding * 2);
            this->setMinimumWidth(Rect.width() + Padding * 2);
        }
    protected:
        virtual void mousePressEvent(QMouseEvent* )override{
            bool Ok = true;
            auto Font = QFontDialog::getFont(&Ok, CurrentFont, this, "Choose A Font");
            if(Ok){
                this->SetFont(Font);
            }
        }

        virtual void paintEvent(QPaintEvent* Event)override{
            QPainter Painter(this);
            Painter.setRenderHint(QPainter::Antialiasing);
            QPen Pen;
            Pen.setColor(Qt::black);
            Pen.setWidth(2);
            Pen.setCapStyle(Qt::PenCapStyle::RoundCap);

            auto Rect = Event->rect();
            auto TargetRect = PaddingIn(Rect, 2);
//            auto TargetRect = GetTextRect();
            QPainterPath Path;
            Path.addRoundedRect(TargetRect, 5, 5);
            Painter.fillPath(Path, Qt::white);
            Painter.setPen(Pen);
            Painter.drawPath(Path);

            Painter.setFont(CurrentFont);
            Painter.drawText(TargetRect, Qt::AlignCenter, GetText());
//            QTextOption TextOp;
//            TextOp.setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
//            QTextLayout layout(GetText(), CurrentFont);
//            SetupTextLayout(&layout);
//            layout.setTextOption(TextOp);
//            Pen.setColor(Qt::black);
//            Painter.setPen(Pen);
//            layout.draw(&Painter, QPoint(0, 0));
        }
    public:
        KFontPicker(){
            UpdateSize();
        }

        void SetFont(const QFont& Font){
            if(this->CurrentFont != Font){
                this->CurrentFont = Font;
                emit FontChangedSignal(Font);
                UpdateSize();
                this->update();
            }
        }
    };
}
