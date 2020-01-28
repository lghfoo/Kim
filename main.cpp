#include<QApplication>
#include"./controller/ApplicationController.hpp"
#include<QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}

// 2020/1/28 18:33
// Done:
// 1、添加帮助界面
// 2、文本：希腊符号
// 3、增加保存提示功能
// 4、图片拖到场景功能
// 9、图片复制到节点、场景功能。
// 14、在鼠标处添加item
// TODO:
// 5、group选中的connection
// 6、自己不能连接自己
// 7、节点折叠处理循环、重复、多个src、以及相对位置
// 8、scroll到指点item
// 10、connection支持文本输入
// 11、工具面板
// 12、+/-折叠
// 13、序列化，添加对折叠的处理
// 15、修复Item缩放时场景像素残留bug
// 16、ImageItem Format
// 17、Text item cache bounding
// 18、折叠节点
// 19、选中节点的所有孩子
// 20、选中连接的所有孩子

// 2020/1/27 18:33
// Done:
// 1、节点折叠
// 2、折叠时更新选中状态
// TODO:
// 1、增加保存提示功能
// 2、图片复制到节点、场景功能。
// 3、图片拖到场景功能
// 4、文本：希腊符号
// 5、group选中的connection
// 6、自己不能连接自己
// 7、节点折叠处理循环和重复、保存选中状态以及相对位置
// 8、scroll到指点item
// 9、快捷键提示
// 10、connection支持文本输入
// 11、工具面板
// 12、+/-折叠
// 13、序列化，添加对折叠的处理
// 14、在鼠标处添加item

// 2020/1/26 22:03
// TODO:
// 1、增加保存提示功能
// 2、图片复制到节点、场景功能。
// 3、图片拖到场景功能
// 4、文本：希腊符号
// 5、节点折叠
// 6、group选中的connection
// 7、自己不能连接自己
// 8、节点折叠处理循环和重复、保存选中状态以及相对位置
// 9、scroll到指点item
// 10、快捷键提示
