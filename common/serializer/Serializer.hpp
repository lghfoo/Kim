#pragma once
#include"../../controller/CanvasController.hpp"
#include"../../controller/ItemController.hpp"
#include"../../controller/ConnectionController.hpp"
namespace Kim {
    namespace PropertyName {
        namespace Base {
            const static QString Identity = "Identity";
            const static QString Alias = "Alias";
            const static QString CreatedAt = "CreatedAt";
            const static QString LastModifiedAt = "LastModifiedAt";
            const static QString Position = "Position";
            const static QString FoldConnectionCount = "FoldConnectionCount";
            const static QString Collapsed = "Collapsed";
            const static QString ParentPosWhenCollapse = "ParentPosWhenCollapse";
        }

        namespace Text {
            const static QString Content = "Content";
        }

        namespace Image {
            const static QString Content = "Content";
        }

        namespace Connection {
            const static QString FromID = "FromID";
            const static QString ToID = "ToID";
            const static QString Collapsed = "Collapsed";
        }
    }
    namespace ItemTypeName  {
        const static QString TextItem = "TextItem";
        const static QString ImageItem = "ImageItem";
        const static QString BaseItem = "BaseItem";
    }
    class KSerializer : public QObject{
        Q_OBJECT
    signals:
        void TotalProgressSignal(int Total);
        void ProgressSignal(int Current);
        void FinishedSignal();
    public:
        /**
         * @brief Serialize
         * @param CanvasController: the controller to serialize from, should be not null
         * @param OutputPath: output path
         */
        virtual void Serialize(KCanvasController* CanvasController, const QString& OutputPath) = 0;
        /**
         * @brief Deserialize
         * @param InputPath: input path
         * @param CanvasController: the controller to serialize to, should be not null
         */
        virtual void Deserialize(const QString& InputPath, KCanvasController* CanvasController, const QVariant& CanvasID = QVariant()) = 0;
        virtual ~KSerializer(){}
    };
}
