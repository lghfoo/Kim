#include<QApplication>
#include"./controller/ApplicationController.hpp"
#include<QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}

// 2020/1/26 22:03
// TODO:
// 1、增加保存提示功能
// 2、图片复制到节点、场景功能。
// 3、图片拖到场景功能
// 4、文本：阿拉伯符号
// 5、节点折叠
// 6、group选中的connection
