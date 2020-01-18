#pragma once
#include<QString>
namespace Kim {
    class DragDropProtocol{
    public:
        struct MimeType{
            static QString& DragingConnection(){
                static QString Type = "DragingConnection";
                return Type;
            }
            static QString& DragingImage(){
                static QString Type = "DragingImage";
                return Type;
            }
        };
    };
}
