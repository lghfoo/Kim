#include<QApplication>
#include"./controller/ApplicationController.hpp"
#include<QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}

// 2020/1/27 18:33
// Done:
// 1、节点折叠
// TODO:
// 1、增加保存提示功能
// 2、图片复制到节点、场景功能。
// 3、图片拖到场景功能
// 4、文本：阿拉伯符号
// 5、group选中的connection
// 6、自己不能连接自己
// 7、节点折叠处理循环和重复、保存选中状态以及相对位置
// 8、scroll到指点item
// 9、快捷键提示
// 10、connection支持文本输入
// 11、工具面板

// 2020/1/26 22:03
// TODO:
// 1、增加保存提示功能
// 2、图片复制到节点、场景功能。
// 3、图片拖到场景功能
// 4、文本：阿拉伯符号
// 5、节点折叠
// 6、group选中的connection
// 7、自己不能连接自己
// 8、节点折叠处理循环和重复、保存选中状态以及相对位置
// 9、scroll到指点item
// 10、快捷键提示
