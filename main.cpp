﻿#include<QApplication>
#include"./controller/ApplicationController.hpp"
#include<QDebug>
#include<QtSql>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Kim::KApplicationController::Startup();
    return a.exec();
}
//////////////////////////////// 2020/2/23 16:54 ////////////////////////////////
// Done:
// 54、update group

// TODO:
// 55、group-collapse-ungroup bug;
// 56、group-group bug;
// 53、修正删除时偶尔crash的bug
// 52、refactor style
// 50、右键松开时选择item类型
// 48、修正connection的bounding rect
// 45、复用text edit的edit dialog
// 43、最小化弹出的tab时不太正常
// 42、同步文件名与canvs name：打开时、保存时
// 40、main view hide的时候保持弹出来的widget不要hide
// 41、升级group，保存group的信息，不要在ungroup时丢失。
// 15、修复Item缩放时场景像素残留bug：bounding有问题: 放大场景，频繁点击并拖动场景，光标还有残影
// 35、id管理
// 30、节点引用场景
// 13、文本序列化，添加对折叠的处理
// 10、connection支持文本输入
// 11、工具面板
// 16、ImageItem Format
// 17、Text item cache bounding
// 24、复制、剪切节点、连接
// 25、connection属性面板
// 26、item属性面板
// 27、使用lz4压缩图片数据
// 29、焦点管理并更新选择孩子操作（现在是从item和connection发出signal来进行选择，期望改成从canvs的keyPress事件中根据焦点进行选择）
// 31、snap to grid
// 34、DB:保存到一个已存在的文件时清空文件
// 21、重构Serializer
// 23、重构Mime
// 35、重构sql语句
// 36、选择某条路径expand
// 37、group in out connection
// 38、close console, redirect log to file

//////////////////////////////// 2020/2/5 18:09 ////////////////////////////////
// DONE:
// 49、connection/item shape line

//////////////////////////////// 2020/2/3 18:09 ////////////////////////////////
// DONE:
// 51、text item style

//////////////////////////////// 2020/2/2 15:54 ////////////////////////////////
// Done:
// 46、保存时弹出框置顶
// 47、文本Item double LL
// 5、group选中的connection

//////////////////////////////// 2020/2/1 ??:?? ////////////////////////////////
// Done:
// 28、多场景管理
// 8、scroll到指点item: QGraphicsView自带centerOn方法可以实现scroll到指点item
// 40、extract canvas to top windows: float tab widget
// 16、修复折叠+group保存时parameter count mismatch的bug：insert group少了')'
// 39、修复多canvs时save到同一文件的bug
// 44、置顶文本item的text edit

//////////////////////////////// 2020/1/31 10:33 ////////////////////////////////
// Done:
// 30、group节点（右侧big plus）
// ??、DB Serializer

// Partial:
// 15、修复Item缩放时场景像素残留bug：bounding有问题: 放大场景，频繁点击并拖动场景，光标还有残影

//////////////////////////////// 2020/1/30 10:14 ////////////////////////////////
// Done:
// 22、添加数据库Serializer
// 32、使用long long作id
// 33、item的bounding改变时更新fold mark

// Partial:
// 30、group节点（右侧big plus）

//////////////////////////////// 2020/1/29 11:02 ////////////////////////////////
// Done:
// 30、解决弹出模态框并关闭之后移动item会出现选框的bug:在emit之前ungrab mouse
// 12、+/-折叠
// 7、节点折叠处理循环、重复、多个src、以及相对位置
// 18、折叠节点所有连接

// 2020/1/28 16:51
// Done:
// 1、添加帮助界面
// 2、文本：希腊符号
// 3、增加保存提示功能
// 4、图片拖到场景功能
// 9、图片复制到节点、场景功能。
// 6、自己不能连接自己
// 14、在鼠标处添加item
// 19、选中节点的所有孩子
// 20、选中连接的所有孩子

// 2020/1/27 18:33
// Done:
// 1、节点折叠
// 2、折叠时更新选中状态

// 2020/1/26 22:03
