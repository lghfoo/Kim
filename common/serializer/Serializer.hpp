#pragma once
#include"../../controller/CanvasController.hpp"
#include"../../controller/ItemController.hpp"
#include"../../controller/ConnectionController.hpp"
namespace Kim {
    class KSerializer{
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
        virtual void Deserialize(const QString& InputPath, KCanvasController* CanvasController) = 0;
        virtual ~KSerializer(){}
    };
}
