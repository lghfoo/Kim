#pragma once
#include<QtSql>
#include"Serializer.hpp"
namespace Kim {
    class KDBSerializer : public KSerializer{
    public:
        /**
         * @brief Serialize
         * @param CanvasController: the controller to serialize from, should be not null
         * @param OutputPath: output path
         */
        virtual void Serialize(KCanvasController* CanvasController, const QString& OutputPath) override{
            QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE");//not dbConnection
            DB.setDatabaseName(OutputPath);
            DB.open();
            bool Result = true;
            QSqlQuery Query;
            auto CheckResult = [](bool Result, const QSqlQuery& Query){
                if(!Result){
                    qDebug()<<Query.lastError();
                }
            };
            // Drop Table
            Result = Query.exec("DROP TABLE IF EXISTS 'TextItem'");
            CheckResult(Result, Query);
            Result = Query.exec("DROP TABLE IF EXISTS 'ImageItem'");
            CheckResult(Result, Query);
            Result = Query.exec("DROP TABLE IF EXISTS 'Connection'");
            CheckResult(Result, Query);
            // Create Table
            Result = Query.exec("CREATE TABLE 'TextItem' ("
                                      "'Idenity'	TEXT,       "
                                      "'Alias'	TEXT,           "
                                      "'CreatedAt'	TEXT,       "
                                      "'LastModifiedAt'	TEXT,   "
                                      "'Position'	TEXT,       "
                                      "'Content'	TEXT,       "
                                      "PRIMARY KEY('Idenity')   "
                                                            ")");
            CheckResult(Result, Query);
            Result = Query.exec("CREATE TABLE 'ImageItem' ("
                                      "'Idenity'	TEXT,       "
                                      "'Alias'	TEXT,           "
                                      "'CreatedAt'	TEXT,       "
                                      "'LastModifiedAt'	TEXT,   "
                                      "'Position'	TEXT,       "
                                      "'Content'	BLOB,       "
                                      "PRIMARY KEY('Idenity')   "
                                                            ")");
            CheckResult(Result, Query);
            Result = Query.exec("CREATE TABLE 'Connection' ("
                                      "'FromID'     TEXT,        "
                                      "'ToID'       TEXT        "
                                                            ")");
            CheckResult(Result, Query);
            // Insert Data
            // TODO: use transcation
            const auto& Items = CanvasController->ItemControlleres;
            const auto& Conns = CanvasController->ConnectionControlleres;
            // Insert Item
            auto ToString = [](const QPointF& Point)->QString{
                return QString("%1, %2").arg(Point.x()).arg(Point.y());
            };
            QString TextItemPrepare = "INSERT INTO TextItem VALUES (:Identity, :Alias, :CreatedAt,"
                                      ":LastModifiedAt, :Position, :Content)";
            QString ImageItemPrepare = "INSERT INTO ImageItem VALUES (:Identity, :Alias, :CreatedAt,"
                                      ":LastModifiedAt, :Position, :Content)";
            QString ConnectionPrepare = "INSERT INTO Connection VALUES (:FromID, :ToID)";
            // Insert Text Item
            Query.prepare(TextItemPrepare);
            for(auto Item : Items){
                if(Item->type() == KTextItemView::Type){
                    auto TextItem = static_cast<KTextItemController*>(Item);
                    Query.bindValue(":Identity", QVariant(TextItem->Identity));
                    Query.bindValue(":Alias", QVariant(TextItem->Alias));
                    Query.bindValue(":CreatedAt", QVariant(TextItem->CreatedTime.toString(NormalTimeFormat)));
                    Query.bindValue(":LastModifiedAt", QVariant(TextItem->LastModifiedTime.toString(NormalTimeFormat)));
                    Query.bindValue(":Position", ToString(TextItem->GetView()->pos()));
                    Query.bindValue(":Content", QVariant(TextItem->GetView()->GetContent()));
                    Result = Query.exec();
                    CheckResult(Result, Query);
                }
            }
            // Insert Image Item
            Query.prepare(ImageItemPrepare);
            for(auto Item : Items){
                if(Item->type() == KImageItemView::Type){
                    auto ImageItem = static_cast<KImageItemController*>(Item);
                    Query.bindValue(":Identity", QVariant(ImageItem->Identity));
                    Query.bindValue(":Alias", QVariant(ImageItem->Alias));
                    Query.bindValue(":CreatedAt", QVariant(ImageItem->CreatedTime.toString(NormalTimeFormat)));
                    Query.bindValue(":LastModifiedAt", QVariant(ImageItem->LastModifiedTime.toString(NormalTimeFormat)));
                    Query.bindValue(":Position", ToString(ImageItem->GetView()->pos()));
                    Query.bindValue(":Content", ImageItem->GetItemView<KImageItemView>()->GetImageAsByteArray());
                    Result = Query.exec();
                    CheckResult(Result, Query);
                }
            }
            // Insert Connection
            Query.prepare(ConnectionPrepare);
            for(auto Conn : Conns){
                Query.bindValue(":FromID", Conn->GetSrcItemController()->Identity);
                Query.bindValue(":ToID", Conn->GetDstItemController()->Identity);
                Result = Query.exec();
                CheckResult(Result, Query);
            }
            emit FinishedSignal();
        }
        /**
         * @brief Deserialize
         * @param InputPath: input path
         * @param CanvasController: the controller to serialize to, should be not null
         */
        virtual void Deserialize(const QString& InputPath, KCanvasController* CanvasController) override{

        }
    };
}
