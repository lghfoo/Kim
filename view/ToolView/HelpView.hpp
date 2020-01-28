#pragma once
#include<QWidget>
#include<QTextEdit>
#include<QLabel>
#include<QLayout>
#include<QGridLayout>
#include<QHBoxLayout>
#include<QVBoxLayout>
namespace Kim {
    class KHelpView : public QWidget{
    public:
        KHelpView(){
            QVBoxLayout* Layout = new QVBoxLayout;
            QTextEdit* HelpEdit = new QTextEdit;
            HelpEdit->setReadOnly(true);
            HelpEdit->setText(QString::fromLocal8Bit("WASD：上下左右移动目标\n"
                                                     "H：折叠节点/连接\n"
                                                     "P：切换节点添加位置\n"
                                                     "M：切换移动对象\n"
                                                     "1：添加文本节点\n"
                                                     "2：添加图像节点\n"
                                                     "Ctrl + V：粘贴\n"
                                                     "Ctrl + S：保存\n"
                                                     "Ctrl + O：打开.kim文件\n"
                                                     "Alt  + R：切换输入状态"));
            Layout->addWidget(HelpEdit);
            this->setLayout(Layout);
        }
    };

}
