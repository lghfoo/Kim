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
            bool TranscationResult = DB.transaction();
            if(!TranscationResult)
                qDebug()<<DB.lastError();

            bool Result = true;
            QSqlQuery Query;
            auto CheckResult = [](bool Result, const QSqlQuery& Query){
                if(!Result){
                    qDebug()<<Query.lastError();
                }
            };
            // Drop Table
            Result = Query.exec("DROP TABLE IF EXISTS 'BaseItem'");
            CheckResult(Result, Query);
            Result = Query.exec("DROP TABLE IF EXISTS 'TextItem'");
            CheckResult(Result, Query);
            Result = Query.exec("DROP TABLE IF EXISTS 'ImageItem'");
            CheckResult(Result, Query);
            Result = Query.exec("DROP TABLE IF EXISTS 'Connection'");
            CheckResult(Result, Query);
            // Create Table
            Result = Query.exec("CREATE TABLE 'BaseItem' ("
                                "'Identity'              INTEGER,       "
                                "'Alias'                TEXT,           "
                                "'CreatedAt'            TEXT,       "
                                "'LastModifiedAt'       TEXT,   "
                                "'Position'             TEXT,       "
                                "'FoldConnectionCount'  INTEGER,"
                                "'Collapsed'            CHAR(2),"
                                "'ParentPosWhenCollapse' TEXT,"
                                "PRIMARY KEY('Identity')   "
                                                      ")");
            Result = Query.exec("CREATE TABLE 'TextItem' ("
                                      "'Identity'    INTEGER,       "
                                      "'Content'	TEXT,       "
                                      "PRIMARY KEY('Identity')   "
                                                            ")");
            CheckResult(Result, Query);
            Result = Query.exec("CREATE TABLE 'ImageItem' ("
                                      "'Identity'   INTEGER,       "
                                      "'Content'	BLOB,       "
                                      "PRIMARY KEY('Identity')   "
                                                            ")");
            CheckResult(Result, Query);
            Result = Query.exec("CREATE TABLE 'Connection' ("
                                      "'FromID'     TEXT,        "
                                      "'ToID'       TEXT,        "
                                      "'Collapsed'  CHAR(2)     "
                                                            ")");
            CheckResult(Result, Query);
            // Insert Data
            // TODO: use transcation
            const auto& Items = CanvasController->ItemControlleres;
            const auto& Conns = CanvasController->ConnectionControlleres;
            // Insert Item
            auto PointToString = [](const QPointF& Point)->QString{
                return QString("%1, %2").arg(Point.x()).arg(Point.y());
            };
            auto BoolToChar = [](bool Bool) -> QChar{
                return Bool? QChar('Y') : QChar('N');
            };
            QString BaseItemPrepare = "INSERT INTO BaseItem VALUES (:Identity, :Alias, :CreatedAt,"
                                      ":LastModifiedAt, :Position, :FoldConnectionCount, :Collapsed,"
                                      ":ParentPosWhenCollapse)";
            QString TextItemPrepare = "INSERT INTO TextItem VALUES (:Identity, :Content)";
            QString ImageItemPrepare = "INSERT INTO ImageItem VALUES (:Identity, :Content)";
            QString ConnectionPrepare = "INSERT INTO Connection VALUES (:FromID, :ToID, :Collapsed)";
            // Insert Base Item
            Query.prepare(BaseItemPrepare);
            for(auto Item : Items){
                Query.bindValue(":Identity", Item->Identity);
                Query.bindValue(":Alias", Item->Alias);
                Query.bindValue(":CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
                Query.bindValue(":LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
                Query.bindValue(":Position", PointToString(Item->GetView()->pos()));
                Query.bindValue(":FoldConnectionCount", Item->FoldConnectionCount);
                Query.bindValue(":Collapsed", BoolToChar(Item->Collapsed));
                Query.bindValue(":ParentPosWhenCollapse", PointToString(Item->ParentPosWhenCollapse));
                Result = Query.exec();
                CheckResult(Result, Query);
            }
            // Insert Text Item
            Query.prepare(TextItemPrepare);
            for(auto Item : Items){
                if(Item->type() == KTextItemView::Type){
                    auto TextItem = static_cast<KTextItemController*>(Item);
                    Query.bindValue(":Identity", QVariant(TextItem->Identity));
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
                Query.bindValue(":Collapsed", BoolToChar(Conn->IsCollapsed()));
                Result = Query.exec();
                CheckResult(Result, Query);
            }
            if(TranscationResult)
                if(!DB.commit())
                    qDebug()<<DB.lastError();
            DB.close();
            emit FinishedSignal();
        }
        /**
         * @brief Deserialize
         * @param InputPath: input path
         * @param CanvasController: the controller to serialize to, should be not null
         */
        virtual void Deserialize(const QString& InputPath, KCanvasController* CanvasController) override{
            QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE");//not dbConnection
            DB.setDatabaseName(InputPath);
            bool Result = true;
            Result = DB.open();
            if(!Result)qDebug()<<DB.lastError();
            QSqlQuery Query;
            auto CheckResult = [](bool Result, const QSqlQuery& Query){
                if(!Result){
                    qDebug()<<Query.lastError();
                }
            };
            auto PointFromString = [](const QString& Point)->QPointF{
                auto List = Point.split(',');
                return QPointF(List[0].trimmed().toDouble(), List[1].trimmed().toDouble());
            };
            auto BoolFromString = [](const QString& String) -> bool{
                return String == QString("Y")? true : false;
            };
            auto ReadBaseInfo = [&](KItemController* Item, const QSqlQuery& Query){
                Item->Identity = Query.value("Identity").toLongLong();
                Item->Alias = Query.value("Alias").toString();
                Item->CreatedTime = QDateTime::fromString(Query.value("CreatedAt").toString(), NormalTimeFormat);
                Item->LastModifiedTime = QDateTime::fromString(Query.value("LastModifiedAt").toString(), NormalTimeFormat);
                Item->GetView()->setPos(PointFromString(Query.value("Position").toString()));
                Item->FoldConnectionCount = Query.value("FoldConnectionCount").toString().toInt();
                Item->SetCollapsed(BoolFromString(Query.value("Collapsed").toString()));
                Item->ParentPosWhenCollapse = PointFromString(Query.value("ParentPosWhenCollapse").toString());
            };
            Query = QSqlQuery("SELECT * FROM BaseItem NATURAL JOIN TextItem");
            while(Query.next()){
                KTextItemController* Item = static_cast<KTextItemController*>(CanvasController->CreateAndAddItemController(KTextItemView::Type));
                ReadBaseInfo(Item, Query);
                Item->GetView()->SetContent(Query.value("Content").toString());
            }
            Query = QSqlQuery("SELECT * FROM BaseItem NATURAL JOIN ImageItem");
            while (Query.next()) {
                KImageItemController* Item = static_cast<KImageItemController*>(CanvasController->CreateAndAddItemController(KImageItemView::Type));
                ReadBaseInfo(Item, Query);
                Item->GetItemView<KImageItemView>()->SetContent(Query.value("Content").toByteArray());
            }
            Query = QSqlQuery("SELECT * FROM Connection");
            while (Query.next()) {
                KConnectionController* Conn = CanvasController->CreateAndAddConnectionController();
                qint64 FromID = Query.value("FromID").toLongLong();
                qint64 ToID = Query.value("ToID").toLongLong();
                bool Collapsed = BoolFromString(Query.value("Collapsed").toString());
                auto SrcItem = CanvasController->GetItemByIdentity(FromID);
                auto DstItem = CanvasController->GetItemByIdentity(ToID);
                Conn->SetSrcItemController(SrcItem);
                Conn->SetDstItemController(DstItem);
                Conn->SetCollapsed(Collapsed);
            }
            DB.close();
        }
    };
}
