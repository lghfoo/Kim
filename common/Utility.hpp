#pragma once
#include <QString>
#include <QDateTime>
#include <QDebug>
namespace Kim {
    static const QString IdentityTimeFormat = "yyyy_MM_dd_hh_mm_ss_zzz";
    static const QString NormalTimeFormat = "yyyy/MM/dd hh:mm:ss";
    enum SelectionType{
        All, None, Reverse
    };

    static qint64 CreateID(){
       auto Now = QDateTime::currentDateTime().toString(IdentityTimeFormat).split('_').join("");
       return Now.toLongLong();
    }
}
