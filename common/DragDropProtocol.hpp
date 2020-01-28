#pragma once
#include<QString>
namespace Kim {
    class DragDropProtocol{
    public:
        struct MimeType{
            static QString& DragingConnection(){
                static QString Type = "kim/dragdrop";
                return Type;
            }
            static QString& DragingImage(){
                static QString Type = "kim/image";
                return Type;
            }            
        };
    };
}
