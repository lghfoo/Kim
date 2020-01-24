#pragma once
#include<QGraphicsItem>
#include<QGraphicsTextItem>
#include<QGraphicsSimpleTextItem>
#include<QGraphicsWidget>
#include<QGraphicsProxyWidget>
#include<QPainter>
#include<QGraphicsSceneMouseEvent>
#include<QDrag>
#include<QWidget>
#include<QMimeData>
#include<QDebug>
#include <QTextLayout>
#include<QGraphicsScene>
#include<QGraphicsView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialog>
namespace Kim {
    class KItemView : public QObject{
        Q_OBJECT
    signals:
        void StartDragDropSignal();
        void EndDragDropSignal();
        void IgnoreDropSignal();
        void PosChangedSignal();
    public:
        virtual QGraphicsItem* ToGraphics() = 0;
        virtual QPointF GetCenterPos() const = 0;
        virtual QString GetTypeAsString() const = 0;
    };

    class KQuickTextEdit : public QTextEdit{
        Q_OBJECT
    signals:
        void PressShiftEnterSignal();
    public:
        KQuickTextEdit(const QString& Text){
            this->setPlainText(Text);
            this->moveCursor(QTextCursor::End);
        }
    protected:
        virtual void keyPressEvent(QKeyEvent *e) override{
            if(e->modifiers() & Qt::ShiftModifier
                    && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)){
                emit PressShiftEnterSignal();
            }
            else{
                QTextEdit::keyPressEvent(e);
            }
        }
    };

    class KEditTextDialog : public QDialog{
        Q_OBJECT
    private:
        KQuickTextEdit* TextEdit = nullptr;
    signals:
        void OKSignal();
    public slots:
        void OnCancel(){
            this->close();
        }
    public:
        QString GetText(){return TextEdit->document()->toPlainText();}
        KEditTextDialog(const QString& Text = ""){
            QVBoxLayout* RootLayout = new QVBoxLayout;
            TextEdit = new KQuickTextEdit(Text);
            QPushButton* OkButton = new QPushButton(tr("OK"));
            QPushButton* CancelButton = new QPushButton(tr("Cancel"));
            QHBoxLayout* ButtonLayout = new QHBoxLayout;
            ButtonLayout->addWidget(OkButton);
            ButtonLayout->addWidget(CancelButton);
            RootLayout->addWidget(TextEdit);
            RootLayout->addLayout(ButtonLayout);
            this->setLayout(RootLayout);

            connect(OkButton,
                    &QPushButton::clicked,
                    this,
                    &KEditTextDialog::OKSignal);
            connect(CancelButton,
                    &QPushButton::clicked,
                    this,
                    &KEditTextDialog::OnCancel);
            connect(TextEdit,
                    &KQuickTextEdit::PressShiftEnterSignal,
                    this,
                    &KEditTextDialog::OKSignal);
        }
    };
    class KTextItemView: public KItemView, public QGraphicsItem  {
        Q_OBJECT
    private:
        qreal Padding = 16.0;
        QString Text = "";
        QString PromptText = "Please Enter Text....";
        QFont Font = QFont("Times", 10);
    signals:
        void EditSignal();
    public:
        enum {Type = UserType + 1};
        static bool IsInvalidChar(const QChar& Char){
            return Char < 32
                    && Char != '\n'
                    && Char != '\r'
                    && Char != '\t';
        }
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)override{
            if(change == QGraphicsItem::GraphicsItemChange::ItemPositionHasChanged){
                emit PosChangedSignal();
            }
            else if(change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged){
                if(this->isSelected()){
                    this->setFocus(Qt::FocusReason::NoFocusReason);
                }
            }
            return QGraphicsItem::itemChange(change, value);
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
        QRectF GetTextSize() const{
            QRectF br;
            const QString& Text = this->Text.isEmpty()? this->PromptText : this->Text;
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
        static QRectF ComputeTextSize(const QString& Text, const QFont& Font){
            static QGraphicsSimpleTextItem* Item = new QGraphicsSimpleTextItem;
            Item->setText(Text);
            Item->setFont(Font);
            return Item->boundingRect();
        }
        void SetText(const QString& Text){
            this->prepareGeometryChange();
            this->Text = Text;
            this->update();
        }
        QString GetText()const{
            return Text;
        }
        QString GetPromptText()const{
            return PromptText;
        }
        void AppendText(const QString& Text){
            this->prepareGeometryChange();
            this->Text.append(Text);
            this->update();
        }
        void DeleteLast(){
            if(this->Text.isEmpty())return;
            this->SetText(this->Text.remove(this->Text.size() - 1, 1));
        }
        virtual QRectF boundingRect() const override{
            QRectF TextSize = GetTextSize();
            qreal Width = TextSize.width() + Padding,
                Height = TextSize.height() + Padding,
                TempX = -Width / 2.0,
                TempY = -Height / 2.0;
            return QRectF(TempX, TempY, Width, Height);
        }
        virtual QPointF GetCenterPos() const override{
            return this->pos();
        }
        KTextItemView(){
            this->setAcceptDrops(true);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsFocusable);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemSendsGeometryChanges);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemAcceptsInputMethod);

        }
        virtual QGraphicsItem* ToGraphics() override{
            return this;
        }
        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override{
            painter->setFont(Font);
            QPen Pen(Qt::black);
            painter->setPen(Pen);
            QPainterPath Path;
            const QRectF& Bounding = this->boundingRect();
            Path.moveTo(Bounding.topLeft());
            Path.addRoundedRect(Bounding, 5.0, 5.0);
            painter->fillPath(Path, QBrush(Qt::white));

            QString FinalText = Text;
            if(Text.isEmpty()){
                FinalText = PromptText;
                Pen.setColor(Qt::darkGray);
                painter->setPen(Pen);
            }
            QString tmp = FinalText;
            tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
            QTextLayout layout(tmp, Font);
            SetupTextLayout(&layout);
            layout.draw(painter, QPointF(boundingRect().x() + Padding/2.0, boundingRect().y() + Padding/2.0));

            Pen.setColor(Qt::black);
            if(this->isSelected()){
                Pen.setWidth(3);
            }
            painter->setPen(Pen);
            painter->drawRoundedRect(this->boundingRect(), 5.0, 5.0);

        }

        int type() const override { return Type; }

        virtual QString GetTypeAsString() const override{return QString("TextItem");}

        //////////////////////////////// Events ////////////////////////////////
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event)override{
            emit EndDragDropSignal();
            event->acceptProposedAction();
        }

        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override{
            if(event->modifiers() & Qt::ControlModifier){
                 emit StartDragDropSignal();
                QMimeData* Data = new QMimeData;
                QDrag* Drag = new QDrag(event->widget());
                Drag->setMimeData(Data);
                Qt::DropAction DropAction = Drag->exec();
                if(DropAction == Qt::IgnoreAction){
                    emit IgnoreDropSignal();
                }
            }
            else{
                 QGraphicsItem::mouseMoveEvent(event);
            }
        }

        virtual void keyPressEvent(QKeyEvent *event)override{
            qDebug()<<event->text()<<event->key();
            // shift+enter open edit window
            if(event->modifiers() & Qt::ShiftModifier && (event->key() == Qt::Key_Enter ||
                    event->key() == Qt::Key_Return)){
                emit EditSignal();
                return;
            }
            // filter all modifiers
            if(event->modifiers() != Qt::NoModifier){
                event->setAccepted(false);
                return;
            }
            // normal input
            if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
                this->AppendText(QString(QChar::LineSeparator));
            }
            else if(event->key() == Qt::Key_Backspace){
                this->DeleteLast();
            }
            else{
                if(event->text().isEmpty() || IsInvalidChar(event->text()[0])){
                    event->setAccepted(false);
                    return;
                }
                else{
                    this->AppendText(event->text());
                }
            }
        }

        virtual void inputMethodEvent(QInputMethodEvent *event)override{
//            printf("input method event\n");
            if(!event->commitString().isEmpty()){
                this->AppendText(event->commitString());
            }

        }

        virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const override{
//            printf("input method query\n");
            return QGraphicsItem::inputMethodQuery(query);
        }


//        virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const override{
//            printf("input method query\n");
//        }
    };

   class KTextItemPropertyView{

   };
}
