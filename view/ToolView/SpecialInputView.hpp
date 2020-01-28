#pragma once
#include<QWidget>
#include<QPushButton>
#include<QLabel>
#include<QLayout>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QGridLayout>
#include <QLinkedList>
#include <QFile>
#include<QDebug>
#include<QTabWidget>
#include <QKeyEvent>
namespace Kim {
    //////////////////////////////// Greece Input ////////////////////////////////
    struct KGreeceLetter{
        QString UpperCase = "";
        QString LowerCase = "";
        QString PhoneticSymbol = "";
        QString EnglishName = "";
        QString ChineseName = "";
        QString Meaning = "";
    };

    struct KGreeceAlphabet{
        static QLinkedList<KGreeceLetter>& GetAlphabet(){
            static QLinkedList<KGreeceLetter> Alphabet = {};
            if(Alphabet.empty()){
                QFile File(":/text/GreeceAlphabet.txt");
                if(File.open(QIODevice::ReadOnly | QIODevice::Text)){
                    QTextStream Stream(&File);
                    Stream.setCodec("utf-8");
                    while(!Stream.atEnd()){
                        const QString& Line = Stream.readLine();
                        const QStringList& Items = Line.split(",");
                        if(Items.size() < 6)continue;
                        Alphabet.append({
                                           Items[0].trimmed(),
                                            Items[1].trimmed(),
                                            Items[2].trimmed(),
                                            Items[3].trimmed(),
                                            Items[4].trimmed(),
                                            Items[5].trimmed()
                                        });
                    }
                }
                else{
                    qDebug()<<"miss GreeceAlphabet.txt";
                }
            }
            return Alphabet;
        }
    };

    class KGreeceInputView : public QWidget{
        Q_OBJECT
    signals:
        void ClickedSignal(const QString& Text);
    private:
    public:
        KGreeceInputView(){
            this->setWindowFlag(Qt::WindowStaysOnTopHint);
            QGridLayout* Layout = new QGridLayout;
            const QLinkedList<KGreeceLetter>& Alphabet = KGreeceAlphabet::GetAlphabet();
//            int LetterCnt = Alphabet.size() * 2;
            int CountPerLine = 6;
            int Count = 0;
            int Row = 0, Col = 0;
            for(const auto& Letter : Alphabet){
                QPushButton* UpperBtn = new QPushButton(Letter.UpperCase);
                UpperBtn->setToolTip(QString::fromLocal8Bit("音标：") + Letter.PhoneticSymbol + "\n"
                                     + QString::fromLocal8Bit("英文名：") + Letter.EnglishName + "\n"
                                     + QString::fromLocal8Bit("中文名：") + Letter.ChineseName);
                Layout->addWidget(UpperBtn, Row, Col);
                Col++;
                if(Col == CountPerLine){
                    Col = 0;
                    Row++;
                }
                connect(UpperBtn,
                        &QPushButton::clicked,
                        [=]{
                   emit ClickedSignal(Letter.UpperCase);
                });
                QPushButton* LowerBtn = new QPushButton(Letter.LowerCase);
                LowerBtn->setToolTip(QString::fromLocal8Bit("音标：") + Letter.PhoneticSymbol + "\n"
                                     + QString::fromLocal8Bit("英文名：") + Letter.EnglishName + "\n"
                                     + QString::fromLocal8Bit("中文名：") + Letter.ChineseName);
                Layout->addWidget(LowerBtn, Row, Col);
                Col++;
                if(Col == CountPerLine){
                    Col = 0;
                    Row++;
                }
                connect(LowerBtn,
                        &QPushButton::clicked,
                        [=]{
                   emit ClickedSignal(Letter.LowerCase);
                });
                Count++;
            }
            this->setLayout(Layout);
        }

//    protected:
//        virtual void resizeEvent(QResizeEvent* Event)override{
//            qDebug()<<Event->size();
//            QWidget::resizeEvent(Event);
//        }
    };

    //////////////////////////////// Special Input ////////////////////////////////
    class KSpecialInputView : public QTabWidget{
        Q_OBJECT
    signals:
        void InputSignal(const QString& Text);
    private:
        KGreeceInputView* GreeceView = new KGreeceInputView;
    public:
        KSpecialInputView(){
            this->addTab(GreeceView, QString::fromLocal8Bit("希腊字母"));
            connect(GreeceView,
                    &KGreeceInputView::ClickedSignal,
                    this,
                    &KSpecialInputView::InputSignal);
        }
    };
}
