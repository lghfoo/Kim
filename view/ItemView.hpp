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
#include <QBuffer>
#include<QApplication>
#include <QClipboard>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include"../common/Utility.hpp"
#include"GraphicsViewBase.hpp"
namespace Kim {
    //////////////////////////////// Item View ////////////////////////////////
    class KItemMark : public QGraphicsObject{
        Q_OBJECT
    public:
        enum KMarkShpae{Plus, Minus, Ellipse};
    signals:
        void ClickedSignal();
    private:
        qreal Radius = 10.0;
        qreal ShapeRadius = Radius * 0.4;
        KMarkShpae MarkShape = Plus;
    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent* Event)override{
            // 必须在clicked signal之前ungrab mouse，
            // 因为可能在signal后this已经被delete了
            this->ungrabMouse();
            emit ClickedSignal();
        }
    public:
        KItemMark(KMarkShpae MarkShape = Plus) : MarkShape(MarkShape){

        }

        virtual ~KItemMark() override{
        }

        void SetRadius(qreal Radius){
            this->Radius = Radius;
            this->ShapeRadius = Radius * 0.4;
        }

        void SetMarkShape(KMarkShpae MarkShape){
            this->MarkShape = MarkShape;
        }

        virtual void paint(QPainter* Painter,
                   const QStyleOptionGraphicsItem* Options,
                   QWidget* Widget = nullptr)override{
            QPen Pen;
            Pen.setWidth(2);
            Pen.setCapStyle(Qt::PenCapStyle::RoundCap);
            Painter->setPen(Pen);
            const QRectF Bounding = GetBounding();
            QPainterPath Path;
            Path.moveTo(Bounding.topLeft());
            Path.addEllipse(Bounding);
            Painter->fillPath(Path, Qt::white);
            Painter->drawEllipse(Bounding);
            const auto& Center = Bounding.center();
            switch (MarkShape) {
            case Plus:{
                QLineF VLine = {Center - QPointF{0, ShapeRadius},
                                Center + QPointF{0, ShapeRadius}};
                Painter->drawLine(VLine);
                // do not break;
            }
            case Minus:{
                QLineF HLine = {Center - QPointF{ShapeRadius, 0},
                                Center + QPointF{ShapeRadius, 0}};
                Painter->drawLine(HLine);
                break;
            }
            case Ellipse:{
                QPointF LeftTop = Center - QPointF{ShapeRadius, ShapeRadius};
                QPainterPath EllipsePath;
                EllipsePath.moveTo(LeftTop);
                EllipsePath.addEllipse(QRectF(LeftTop.x(), LeftTop.y(), ShapeRadius * 2, ShapeRadius * 2));
                Painter->fillPath(EllipsePath, Qt::black);
                break;
            }
            }

        }

        QRectF GetBounding()const{
            return QRectF(
                        -Radius,
                        -Radius,
                        2 * Radius,
                        2 * Radius
                        );
        }

        virtual QRectF boundingRect()const override{
            const QRectF& Bounding = GetBounding();
            qreal Padding = 2.0;
            qreal W = Bounding.width() + Padding * 2, H = Bounding.height() + Padding * 2;
            return QRectF(-W/2, -H/2, W, H);
        }
    };
    class KItemView : public KGraphicsViewBase{
        Q_OBJECT
    signals:
        void StartDragDropSignal();
        void EndDragDropSignal();
        void IgnoreDropSignal();
        void PosChangedSignal();
        void SizeChangedSignal();
        void SelectedAllChildrenSignal(SelectionType);
    public:
        KItemView(){
            this->setAcceptDrops(true);
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);
        }
        virtual QString GetTypeAsString() const = 0;
        virtual QString GetContent() {return "";}
        virtual void SetContent(const QString& Content) {Q_UNUSED(Content)}
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)override{
            if(change == QGraphicsItem::GraphicsItemChange::ItemPositionHasChanged){
                emit PosChangedSignal();
            }
            else if(change == QGraphicsItem::GraphicsItemChange::ItemSelectedHasChanged){
                if(this->isSelected()){
                    this->setFocus(Qt::FocusReason::NoFocusReason);
                }
            }
            return KGraphicsViewBase::itemChange(change, value);
        }
    protected:
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override{
            if(event->buttons() & Qt::RightButton){
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
        virtual bool sceneEvent(QEvent* Event) override{
            switch (Event->type()) {
            case QEvent::GraphicsSceneDrop:{
                KGraphicsViewBase::sceneEvent(Event);
                QGraphicsSceneDragDropEvent* E = static_cast<QGraphicsSceneDragDropEvent*>(Event);
                E->acceptProposedAction();
                E->setAccepted(true);
                emit EndDragDropSignal();
                return true;
            }
            case QEvent::KeyPress:{
                QKeyEvent* KeyEvent = static_cast<QKeyEvent*>(Event);
                if(KeyEvent->key() == Qt::Key_L){
                    KGraphicsViewBase::sceneEvent(Event);
                    if(!Event->isAccepted()){
                        if(KeyEvent->modifiers() & Qt::AltModifier){
                            emit SelectedAllChildrenSignal(SelectionType::Reverse);
                        }
                        else if(KeyEvent->modifiers() & Qt::ShiftModifier){
                            emit SelectedAllChildrenSignal(SelectionType::None);
                        }
                        else{
                            emit SelectedAllChildrenSignal(SelectionType::All);
                        }
                        Event->setAccepted(true);
                    }
                    return true;
                }
                break;
            }
            default:
                break;
            }
            return KGraphicsViewBase::sceneEvent(Event);
        }
    };
    //////////////////////////////// Text Item ////////////////////////////////
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


        virtual void showEvent(QShowEvent * e) override{
            this->moveCursor(QTextCursor::End);
            QTextEdit::showEvent(e);
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
        void SetText(const QString& Text){
            TextEdit->setText(Text);
        }
    };
    class KTextItemPropertyController;
    class KTextItemPropertyView;
    class KTextItemView: public KItemView {
        Q_OBJECT
        friend class KTextItemPropertyView;
        friend class KTextItemPropertyController;
    public:
        enum {Type = ViewType::TextItemType};
        enum StylePreset{Green, Orange, Red, Yellow, Black, Blue, Steel, PresetCount};
        struct KStyle{
            QFont Font = QFont("Times", 10);
            QColor BorderColor = Qt::black;
            QColor TextColor = Qt::black;
            QColor BackgroundColor = Qt::white;
        };
    private:
        qreal Padding = 16.0;
        QString Text = "";
        QString PromptText = "Please Enter Text....";
        KStyle Style;
//        Qt::Alignment TextAlignment = Qt::AlignCenter;
    signals:
        void EditSignal();
    public:
        static QMap<int, KStyle>& GetStylePresets(){
            static QMap<int, KStyle> Presets{
                {Green, {QFont("Times", 10), QColor(Qt::green).darker(), QColor(Qt::green).darker(), Qt::white}},
                {Orange, {QFont("Times", 10), QColor(255,165,0).darker(), QColor(255,165,0).darker(), Qt::white}},
                {Red, {QFont("Times", 10), Qt::red, Qt::red, Qt::white}},
                {Yellow, {QFont("Times", 10), QColor(Qt::yellow).darker(), QColor(Qt::yellow).darker(), Qt::white}},
                {Black, {QFont("Times", 10), Qt::black, Qt::black, Qt::white}},
                {Blue, {QFont("Times", 10), Qt::blue, Qt::blue, Qt::white}},
                {Steel, {QFont("Times", 10), QColor(70,130,180).darker(), QColor(70,130,180).darker(), Qt::white}}
            };
            return Presets;
        }

        static KStyle GetStyle(int StyleEnum){
            return GetStylePresets().value(StyleEnum);
        }

        void SetText(const QString& Text){
            this->prepareGeometryChange();
            this->Text = Text;
            this->update();
            emit SizeChangedSignal();
        }

        KStyle GetStyle(){
            return Style;
        }

        void SetStyle(const KStyle& Style){
            this->prepareGeometryChange();
            this->Style = Style;
            this->update();
        }

        void SetText(const QMimeData* MimeData){
            if(MimeData->hasUrls()){
                const QList<QUrl>& Urls = MimeData->urls();
                const QStringList& SupportedFormats = GetSupportedFormats();
                bool ShouldBreak = false;
                for(const QUrl& Url : Urls){
                    const QString UrlString = Url.toDisplayString();
                    const QString UrlStringUpper = UrlString.toUpper();
                    for(const QString& Format : SupportedFormats){
                        if(UrlStringUpper.endsWith(Format)){
                            if(Url.isLocalFile()){
                                QFile TextFile(Url.toLocalFile());
                                if(TextFile.open(QIODevice::ReadOnly|QIODevice::Text)){
                                    QTextStream Stream(&TextFile);
                                    this->SetText(Stream.readAll());
                                    ShouldBreak = true;
                                    TextFile.close();
                                    break;
                                }
                            }
                        }
                    }
                    if(ShouldBreak)break;
                }
            }
            else if(MimeData->hasText()){
                SetText(MimeData->text());
            }
        }

        QString GetText()const{
            return Text;
        }

        QString GetPromptText()const{
            return PromptText;
        }

        QRectF TextBounding()const{
            QRectF TextSize = GetTextSize();
            qreal Width = TextSize.width() + Padding,
                Height = TextSize.height() + Padding,
                TempX = -Width / 2.0,
                TempY = -Height / 2.0;
            return QRectF(TempX, TempY, Width, Height);
        }

        virtual QRectF boundingRect() const override{
            qreal PixelPadding = 1.0;
            const auto& Bounding = TextBounding();
            qreal W = Bounding.width() + PixelPadding * 2;
            qreal H = Bounding.height() + PixelPadding * 2;
            return QRectF(-W/2, -H/2, W, H);
        }

        KTextItemView(){
            this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemAcceptsInputMethod);
        }

        virtual ~KTextItemView()override{
//            qDebug()<<"destroy text item view";
        }

        virtual KGraphicsViewBase* Clone()override{
            KTextItemView* View = new KTextItemView;
            View->setPos(this->pos());
            View->Padding = this->Padding;
            View->Text = this->Text;
            View->PromptText = this->PromptText;
            View->Style = this->Style;
            return View;
        }

        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override{
            painter->setFont(Style.Font);
            QPen Pen;
            painter->setPen(Pen);
            QPainterPath Path;
            const QRectF& Bounding = this->TextBounding();
            Path.moveTo(Bounding.topLeft());
            Path.addRoundedRect(Bounding, 5.0, 5.0);
            painter->fillPath(Path, Style.BackgroundColor);

            QString FinalText = Text;
            Pen.setColor(Style.TextColor);
            if(Text.isEmpty()){
                FinalText = PromptText;
                Pen.setColor(Style.TextColor.lighter());
            }
            painter->setPen(Pen);
            QString tmp = FinalText;
            tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
            QTextLayout layout(tmp, Style.Font);
            SetupTextLayout(&layout);
            layout.draw(painter, QPointF(Bounding.x() + Padding/2.0, Bounding.y() + Padding/2.0));

            Pen.setColor(Style.BorderColor);
            if(this->isSelected()){
                Pen.setWidth(3);
            }
            painter->setPen(Pen);
            painter->drawRoundedRect(Bounding, 5.0, 5.0);

        }

        int type() const override { return Type; }

        virtual QString GetTypeAsString() const override{return QString("TextItem");}

        virtual QString GetContent() override{return this->GetText();}

        virtual void SetContent(const QString& Content) override{this->SetText(Content);}
        //////////////////////////////// Utility ////////////////////////////////
        void AppendText(const QString& Text){
            this->SetText(this->Text + Text);
        }

        void DeleteLast(){
            if(this->Text.isEmpty())return;
            this->SetText(this->Text.remove(this->Text.size() - 1, 1));
        }

        static bool& IsWriteDirect(){
            static bool WriteDirect = false;
            return WriteDirect;
        }

        static bool IsInvalidChar(const QChar& Char){
            return (Char < 32
                    && Char != '\n'
                    && Char != '\r'
                    && Char != '\t')
                    || Char == 127;
        }

        QRectF GetTextSize() const{
            const QString& Text = this->Text.isEmpty()? this->PromptText : this->Text;
            return Kim::GetTextSize(Text, Style.Font);
        }

        static QStringList GetSupportedFormats(){
            const static QStringList SupportedFormats{
                ".TXT",
                ".CPP",
                ".H",
                ".C",
                ".CS",
                ".PY",
                ".HPP",
            };
            return SupportedFormats;
        }

        static bool HasTextData(const QMimeData* MimeData){
            if(MimeData->hasUrls()){
                const QList<QUrl>& Urls = MimeData->urls();
                const QStringList& SupportedFormats = GetSupportedFormats();
                for(const QUrl& Url : Urls){
                    const QString UrlString = Url.toDisplayString();
                    const QString UrlStringUpper = UrlString.toUpper();
                    for(const QString& Format : SupportedFormats){
                        if(UrlStringUpper.endsWith(Format)){
                            if(Url.isLocalFile()){
                                return true;
                            }
                        }
                    }
                }
            }
            if(MimeData->hasText()){
                return true;
            }
            return false;
        }
        //////////////////////////////// Events ////////////////////////////////
        virtual bool sceneEvent(QEvent* Event)override{
            if(Event->type() == QEvent::KeyPress){
                QKeyEvent *k = static_cast<QKeyEvent *>(Event);
                if(k->key() == Qt::Key_Tab){
                    this->keyPressEvent(k);
                    return true;
                }
            }
            return KItemView::sceneEvent(Event);
        }

        virtual void keyReleaseEvent(QKeyEvent* event)override{
            KItemView::keyReleaseEvent(event);
        }

        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)override{
            KItemView::mouseDoubleClickEvent(event);
            emit EditSignal();
        }

        virtual void keyPressEvent(QKeyEvent *event)override{
            // shift+enter open edit window
            if(event->modifiers() & Qt::ShiftModifier && (event->key() == Qt::Key_Enter ||
                    event->key() == Qt::Key_Return)){
                emit EditSignal();
                return;
            }
            // ctrl v
            if(event->modifiers() & Qt::ControlModifier
                    && event->key() == Qt::Key_V){
                QClipboard* Clipboard = QApplication::clipboard();
                auto Data = Clipboard->mimeData();
                this->SetText(Data);
                event->setAccepted(true);
                return;
            }
            if(!IsWriteDirect()){
                event->setAccepted(false);
                return;
            }
            // filter out ctrl & alt modifiers
            if((event->modifiers() & Qt::ControlModifier)
                    ||(event->modifiers() & Qt::AltModifier)){
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
            if(IsWriteDirect() && !event->commitString().isEmpty()){
                this->AppendText(event->commitString());
            }

        }

        virtual void dropEvent(QGraphicsSceneDragDropEvent* Event) override{
            auto MimeData = Event->mimeData();
            SetText(MimeData);
        }
    };
    class KTextItemPropertyView : public QWidget{
        Q_OBJECT
        friend class KTextItemPropertyController;
    signals:
        void AliasChangedSignal(const QString& Alias);
        void FontColorChangedSignal(const QColor& FontColor);
        void BackgroundColorChangedSignal(const QColor& BackgroundColor);
        void BorderColorChangedSignal(const QColor& BorderColor);
        void FontChangedSignal(const QFont& Font);
    private:
        QLineEdit* ItemAliasEdit = new QLineEdit;
        KColorPicker* FontColorPicker = new KColorPicker("Font");
        KColorPicker* BackgroundColorPicker = new KColorPicker("Background");
        KColorPicker* BorderColorPicker = new KColorPicker("Border");
        KFontPicker* FontPicker = new KFontPicker;
        QComboBox* PresetCombo = new QComboBox;
    public:
        KTextItemPropertyView(){
            QGridLayout* Layout = new QGridLayout;
            Layout->addWidget(new QLabel("Alias: "), 0, 0);
            Layout->addWidget(ItemAliasEdit, 0, 1);
            Layout->addWidget(FontColorPicker, 1, 0, 1, 2);
            Layout->addWidget(BackgroundColorPicker, 2, 0, 1, 2);
            Layout->addWidget(BorderColorPicker, 3, 0, 1, 2);
            Layout->addWidget(PresetCombo, 4, 1);
            Layout->addWidget(new QLabel("Font: "), 5, 0);
            Layout->addWidget(FontPicker, 5, 1);
            this->setLayout(Layout);

            PresetCombo->addItem("Black");
            PresetCombo->addItem("Green");
            PresetCombo->addItem("Orange");
            PresetCombo->addItem("Red");
            PresetCombo->addItem("Yellow");
            PresetCombo->addItem("Blue");
            PresetCombo->addItem("Steel");

            connect(ItemAliasEdit, &QLineEdit::textChanged, this, &KTextItemPropertyView::AliasChangedSignal);
            connect(FontColorPicker, &KColorPicker::ColorChangedSignal, this, &KTextItemPropertyView::FontColorChangedSignal);
            connect(BackgroundColorPicker, &KColorPicker::ColorChangedSignal, this, &KTextItemPropertyView::BackgroundColorChangedSignal);
            connect(BorderColorPicker, &KColorPicker::ColorChangedSignal, this, &KTextItemPropertyView::BorderColorChangedSignal);
            connect(FontPicker, &KFontPicker::FontChangedSignal, this, &KTextItemPropertyView::FontChangedSignal);
            connect(PresetCombo, &QComboBox::currentTextChanged, this, &KTextItemPropertyView::SetPreset);
        }
        void SetPreset(const QString& PresetName){
            KTextItemView::KStyle Style;
            if(PresetName == "Black"){
                Style = KTextItemView::GetStyle(KTextItemView::Black);
            }
            else if(PresetName == "Green"){
                Style = KTextItemView::GetStyle(KTextItemView::Green);
            }
            else if(PresetName == "Orange"){
                Style = KTextItemView::GetStyle(KTextItemView::Orange);
            }
            else if(PresetName == "Red"){
                Style = KTextItemView::GetStyle(KTextItemView::Red);
            }
            else if(PresetName == "Yellow"){
                Style = KTextItemView::GetStyle(KTextItemView::Yellow);
            }
            else if(PresetName == "Blue"){
                Style = KTextItemView::GetStyle(KTextItemView::Blue);
            }
            else if(PresetName == "Steel"){
                Style = KTextItemView::GetStyle(KTextItemView::Steel);
            }
            this->FontColorPicker->SetColor(Style.TextColor);
            this->BorderColorPicker->SetColor(Style.BorderColor);
            this->BackgroundColorPicker->SetColor(Style.BackgroundColor);
            this->FontPicker->SetFont(Style.Font);
        }
    };

    //////////////////////////////// Image Item ////////////////////////////////
    class KImageItemView : public KItemView{
        Q_OBJECT
    public: // type def
        enum{Type = ViewType::ImageItemType};
    signals: // signals
        void EditSignal();
    private slots:
    protected slots:
    public slots:

    private:
        QImage Image;
        QString Format = "";
    protected:

    public:
        int type() const override { return Type; }

        QRectF ImageBounding()const{
            static const QSizeF MinSize(128, 128);
            QRectF Bounding(0, 0, MinSize.width(), MinSize.height());
            if(Image.rect().width() > 0 && Image.rect().height() > 0){
                Bounding = Image.rect();
            }
            qreal Padding = 2;
            return QRectF(
                        -(Bounding.width() + Padding) / 2.0,
                        -(Bounding.height() + Padding) / 2.0,
                        Bounding.width() + Padding,
                        Bounding.height() + Padding
                        );
        }

        virtual QRectF boundingRect() const override{
            const auto& Bound = ImageBounding();
            qreal PixelPadding = 1;
            qreal W = Bound.width() + 2 * PixelPadding;
            qreal H = Bound.height() + 2 * PixelPadding;
            return QRectF(-W/2, -H/2, W, H);
        }

        KImageItemView(){
        }

        virtual KGraphicsViewBase* Clone()override{
            KImageItemView* ImageView = new KImageItemView;
            ImageView->Image = this->Image;
            ImageView->Format = this->Format;
            ImageView->setPos(this->pos());
            return ImageView;
        }

        void SetImage(const QImage& Image){
            this->prepareGeometryChange();
            this->Image = Image;
            this->update();
            emit SizeChangedSignal();
//            qDebug()<<"Content"<<this->GetContent();
        }

        void SetImage(const QMimeData* MimeData){
            if(MimeData->hasImage()){
                SetImage(qvariant_cast<QImage>(MimeData->imageData()));
            }
            else if(MimeData->hasUrls()){
                this->Format = "";
                const QList<QUrl>& Urls = MimeData->urls();
                const QStringList& SupportedFormats = GetSupportedFormats();
                bool ShouldBreak = false;
                for(const QUrl& Url : Urls){
                    const QString UrlString = Url.toDisplayString();
                    const QString UrlStringUpper = UrlString.toUpper();
                    for(const QString& Format : SupportedFormats){
                        if(UrlStringUpper.endsWith(Format)){
                            if(Url.isLocalFile()){
                                // todo: save format
//                                this->Format = Format;
//                                this->Format.replace(".", "");
                                SetImage(QImage(Url.toLocalFile()));
                                ShouldBreak = true;
                                break;
                            }
                        }
                    }
                    if(ShouldBreak)break;
                }
            }
        }

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
        {
            QPen Pen(Qt::black);
            painter->setPen(Pen);
            QPainterPath Path;
            const QRectF& Bounding = this->ImageBounding();
            Path.moveTo(Bounding.topLeft());
            Path.addRoundedRect(Bounding, 5.0, 5.0);
            painter->fillPath(Path, QBrush(Qt::white));

            painter->save();
            painter->setClipPath(Path);
            painter->drawImage(Bounding, Image, Image.rect());
            painter->restore();

            Pen.setColor(Qt::black);
            if(this->isSelected()){
                Pen.setWidth(3);
            }
            painter->setPen(Pen);
            painter->drawPath(Path);
        }

        static QStringList GetSupportedFormats(){
            const static QStringList SupportedFormats{
                ".BMP",
                ".GIF",
                ".JPG",
                ".JPEG",
                ".PNG",
                ".PBM",
                ".PGM",
                ".PPM",
                ".XBM",
                ".XPM"
            };
            return SupportedFormats;
        }

        static bool HasImageData(const QMimeData* MimeData){
            if(MimeData->hasImage()){
                return true;
            }
            if(MimeData->hasUrls()){
                const QList<QUrl>& Urls = MimeData->urls();
                const QStringList& SupportedFormats = GetSupportedFormats();
                for(const QUrl& Url : Urls){
                    const QString UrlString = Url.toDisplayString();
                    const QString UrlStringUpper = UrlString.toUpper();
                    for(const QString& Format : SupportedFormats){
                        if(UrlStringUpper.endsWith(Format)){
                            if(Url.isLocalFile()){
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }

        virtual void dropEvent(QGraphicsSceneDragDropEvent* Event) override{
            auto MimeData = Event->mimeData();
            SetImage(MimeData);
        }

        virtual QString GetTypeAsString() const override{
            return QString("ImageItem");
        }

        virtual QString GetContent() override{
            QByteArray Array;
            QBuffer Buffer(&Array);
            Buffer.open(QIODevice::ReadWrite);
            QString Format = this->Format.isEmpty()? "PNG" : this->Format;
            Image.save(&Buffer, Format.toStdString().c_str());
            return Array.toBase64();
        }

        QByteArray GetImageAsByteArray(){
            QByteArray Array;
            QBuffer Buffer(&Array);
            Buffer.open(QIODevice::WriteOnly);
            QString Format = this->Format.isEmpty()? "PNG" : this->Format;
            Image.save(&Buffer, Format.toStdString().c_str());
            return Array;
        }

        virtual void SetContent(const QString& Content)override{
            QByteArray Array = QByteArray::fromBase64(Content.toUtf8());
            QBuffer Buffer(&Array);
            Buffer.open(QIODevice::ReadOnly);
            Image.loadFromData(Array);
        }

        void SetContent(const QByteArray& Content){
            this->Image.loadFromData(Content);
        }

        virtual void keyPressEvent(QKeyEvent* event)override{
            // shift+enter open edit window
            if(event->modifiers() & Qt::ShiftModifier && (event->key() == Qt::Key_Enter ||
                    event->key() == Qt::Key_Return)){
                emit EditSignal();
                return;
            }
            // filter out ctrl & alt modifiers
            if((event->modifiers() & Qt::ControlModifier)
                    ||(event->modifiers() & Qt::AltModifier)){
                if(event->modifiers() & Qt::ControlModifier){
                    if(event->key() == Qt::Key_V){
                        QClipboard* Clipboard = QApplication::clipboard();
                        auto Data = Clipboard->mimeData();
                        this->SetImage(Data);
                        return;
                    }
                }
            }
            event->setAccepted(false);
        }
    };
}
