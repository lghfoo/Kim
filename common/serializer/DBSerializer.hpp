#pragma once
#include<QtSql>
#include"Serializer.hpp"
namespace Kim {
    class KDBSerializer : public KSerializer{
    private:
        const QString ConnectionName = "Kim DBSerializer";
        void CheckResult(bool Result, const QSqlQuery& Query){
            if(!Result){
                qDebug()<<Query.lastError();
            }
        }
        QString PointToString(const QPointF& Point){
            return QString("%1, %2").arg(Point.x()).arg(Point.y());
        }
        QChar BoolToChar(bool Bool){
            return Bool? QChar('Y') : QChar('N');
        }
        QPointF PointFromString(const QString& Point){
            auto List = Point.split(',');
            return QPointF(List[0].trimmed().toDouble(), List[1].trimmed().toDouble());
        }
        bool BoolFromString(const QString& String){
            return String == QString("Y")? true : false;
        }
        QString GroupIDToString(const QVariant& GroupID){
            return GroupID.isNull()? "is null" : QString("=") + QString::number(GroupID.toLongLong());
        }
        void InsertBaseItem(KItemController* Item, QSqlQuery& Query, const QVariant& GroupID = QVariant()){
            Query.bindValue(":Identity", Item->Identity);
            Query.bindValue(":Alias", Item->Alias);
            Query.bindValue(":CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
            Query.bindValue(":LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
            Query.bindValue(":Position", PointToString(Item->GetView()->pos()));
            Query.bindValue(":FoldConnectionCount", Item->FoldConnectionCount);
            Query.bindValue(":Collapsed", BoolToChar(Item->Collapsed));
            Query.bindValue(":ParentPosWhenCollapse", PointToString(Item->ParentPosWhenCollapse));
            Query.bindValue(":GroupItemID", GroupID);
            bool Result = Query.exec();
            CheckResult(Result, Query);
        }
        void InsertTextItem(KTextItemController* TextItem, QSqlQuery& Query){
            Query.bindValue(":Identity", QVariant(TextItem->Identity));
            Query.bindValue(":Content", QVariant(TextItem->GetView()->GetContent()));
            bool Result = Query.exec();
            CheckResult(Result, Query);
        }
        void InsertImageItem(KImageItemController* ImageItem, QSqlQuery& Query){
            Query.bindValue(":Identity", QVariant(ImageItem->Identity));
            Query.bindValue(":Content", ImageItem->GetItemView<KImageItemView>()->GetImageAsByteArray());
            bool Result = Query.exec();
            CheckResult(Result, Query);
        }
        void InsertConnection(KConnectionController* Conn, QSqlQuery& Query, const QVariant& GroupID = QVariant()){
            Query.bindValue(":Identity", QVariant(Conn->GetIdentity()));
            Query.bindValue(":FromID", Conn->GetSrcItemController()->Identity);
            Query.bindValue(":ToID", Conn->GetDstItemController()->Identity);
            Query.bindValue(":Collapsed", BoolToChar(Conn->IsCollapsed()));
            Query.bindValue(":SrcConnected", BoolToChar(Conn->IsSrcConnected()));
            Query.bindValue(":DstConnected", BoolToChar(Conn->IsDstConnected()));
            Query.bindValue(":GroupItemID", GroupID);
            bool Result = Query.exec();
            CheckResult(Result, Query);
        }
        void InsertItems(const QLinkedList<KItemController*>& Items, QSqlQuery& Query, const QVariant& GroupID = QVariant()){
            QString BaseItemPrepare = "INSERT INTO BaseItem VALUES (:Identity, :Alias, :CreatedAt,"
                                      ":LastModifiedAt, :Position, :FoldConnectionCount, :Collapsed,"
                                      ":ParentPosWhenCollapse, :GroupItemID)";
            QString TextItemPrepare = "INSERT INTO TextItem VALUES (:Identity, :Content)";
            QString ImageItemPrepare = "INSERT INTO ImageItem VALUES (:Identity, :Content)";
            Query.prepare(BaseItemPrepare);
            for(auto Item : Items){
                InsertBaseItem(Item, Query, GroupID);
            }
            Query.prepare(TextItemPrepare);
            for(auto Item : Items){
                if(Item->type() == KTextItemView::Type)
                    InsertTextItem(static_cast<KTextItemController*>(Item), Query);
            }
            Query.prepare(ImageItemPrepare);
            for(auto Item : Items){
                if(Item->type() == KImageItemView::Type)
                    InsertImageItem(static_cast<KImageItemController*>(Item), Query);
            }
        }
        void InsertConnections(const QLinkedList<KConnectionController*>& Conns, QSqlQuery& Query, const QVariant& GroupID = QVariant()){
            QString ConnectionPrepare = "INSERT INTO Connection VALUES (:Identity, :FromID, :ToID, :Collapsed, "
                                        ":SrcConnected, :DstConnected, :GroupItemID)";
            Query.prepare(ConnectionPrepare);
            for(auto Conn : Conns){
                InsertConnection(Conn, Query, GroupID);
            }
        }
        void InsertGroups(const QList<KItemGroupController*>& Groups, QSqlQuery& Query){
            QString GroupPrepare = "INSERT INTO ItemGroup VALUES (:GroupItemID, :PosWhenGrouping)";
            Query.prepare(GroupPrepare);
            for(auto Group : Groups){
                Query.bindValue(":GroupItemID", Group->GetGroupID());
                Query.bindValue(":PosWhenGrouping", PointToString(Group->GetPosWhenGrouping()));
                bool Result = Query.exec();
                CheckResult(Result, Query);
            }
        }
    public:
        /**
         * @brief Serialize
         * @param CanvasController: the controller to serialize from, should be not null
         * @param OutputPath: output path
         */
        virtual void Serialize(KCanvasController* CanvasController, const QString& OutputPath) override{
            {
                QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE", ConnectionName);
                DB.setDatabaseName(OutputPath);
                DB.open();
                bool TranscationResult = DB.transaction();
                if(!TranscationResult)
                    qDebug()<<DB.lastError();

                bool Result = true;
                QSqlQuery Query(DB);
                // Drop Table
                Result = Query.exec("DROP TABLE IF EXISTS 'BaseItem'");
                CheckResult(Result, Query);
                Result = Query.exec("DROP TABLE IF EXISTS 'TextItem'");
                CheckResult(Result, Query);
                Result = Query.exec("DROP TABLE IF EXISTS 'ImageItem'");
                CheckResult(Result, Query);
                Result = Query.exec("DROP TABLE IF EXISTS 'Connection'");
                CheckResult(Result, Query);
                Result = Query.exec("DROP TABLE IF EXISTS 'ItemGroup'");
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
                                    "'GroupItemID'          INTEGER,"
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
                                    "'Identity'     INTEGER,"
                                    "'FromID'     INTEGER,        "
                                    "'ToID'       INTEGER,        "
                                    "'Collapsed'  CHAR(2),     "
                                    "'SrcConnected' CHAR(2),     "
                                    "'DstConnected' CHAR(2),     "
                                    "'GroupItemID'  INTEGER"
                                    ")");
                CheckResult(Result, Query);
                Result = Query.exec("CREATE TABLE 'ItemGroup' ("
                                    "'GroupItemID'  INTEGER,"
                                    "'PosWhenGrouping'  TEXT,"
                                    "PRIMARY KEY('GroupItemID')"
                                    ")");
                // Insert Data
                // TODO: use transcation
                const auto& Items = CanvasController->ItemControlleres;
                const auto& Conns = CanvasController->ConnectionControlleres;
                InsertItems(Items, Query);
                InsertConnections(Conns, Query);

                const auto& Groups = CanvasController->GroupControlleres.values();
                for(auto Group : Groups){
                    InsertItems(Group->GetItems(), Query, Group->GetGroupID());
                    InsertConnections(Group->GetAllConnections(), Query, Group->GetGroupID());
                }
                InsertGroups(Groups, Query);
                if(TranscationResult)
                    if(!DB.commit())
                        qDebug()<<DB.lastError();
                DB.close();
            }

            QSqlDatabase::removeDatabase(ConnectionName);
            emit FinishedSignal();
        }
        /**
         * @brief Deserialize
         * @param InputPath: input path
         * @param CanvasController: the controller to serialize to, should be not null
         */
        struct BaseItemRecord{
            qint64 Identity;
            QString Alias;
            QDateTime CreatedAt;
            QDateTime LastModifiedAt;
            QPointF Position;
            int FoldConnectionCount;
            bool Collapsed;
            QPointF ParentPosWhenCollapse;
            qint64 GroupItemID;
            void SetController(KItemController* Controller)const{
                Controller->SetIdentity(Identity);
                Controller->SetAlias(Alias);
                Controller->SetCreatedTime(CreatedAt);
                Controller->SetLastModifiedTime(LastModifiedAt);
                Controller->GetView()->setPos(Position);
                Controller->SetFoldConnectionCount(FoldConnectionCount);
                Controller->SetCollapsed(Collapsed);
                Controller->SetParentPosWhenCollapse(ParentPosWhenCollapse);
            }
        };

        struct TextItemRecord : BaseItemRecord{
            QString Content;
            KTextItemController* ToController()const{
                auto Controller = new KTextItemController;
                SetController(Controller);
                Controller->GetView()->SetContent(Content);
                return Controller;
            }
        };

        struct ImageItemRecord : BaseItemRecord{
            QByteArray Content;
            KImageItemController* ToController()const{
                auto Controller = new KImageItemController;
                SetController(Controller);
                Controller->GetItemView<KImageItemView>()->SetContent(Content);
                return Controller;
            }
        };

        struct ConnectionRecord{
            qint64 Identity;
            qint64 FromID;
            qint64 ToID;
            bool Collapsed;
            bool SrcConnected;
            bool DstConnected;
            qint64 GroupItemID;
            KConnectionController* ToController(
                    const QMap<qint64, KItemController*>& Items
                    )const{
                auto Controller = new KConnectionController;
                Controller->SetIdentity(Identity);
                Controller->SetCollapsed(Collapsed);
                auto SrcItem = Items.find(FromID);
                auto DstItem = Items.find(ToID);
                if(SrcItem != Items.end())
                    Controller->SetSrcItemController(SrcItem.value(), SrcConnected);
                if(DstItem != Items.end())
                    Controller->SetDstItemController(DstItem.value(), DstConnected);
                return Controller;
            }
        };

        struct ItemGroupRecord{
            qint64 GroupItemID;
            QPointF PosWhenGrouping;
            KItemGroupController* ToController()const{
                auto Controller = new KItemGroupController;
                Controller->SetPosWhenGroupping(PosWhenGrouping);
                return Controller;
            }
        };

        void ReadBaseInfo(BaseItemRecord& Record, const QSqlQuery& Query){
            Record.Identity = Query.value("Identity").toLongLong();
            Record.Alias = Query.value("Alias").toString();
            Record.CreatedAt = QDateTime::fromString(Query.value("CreatedAt").toString(), NormalTimeFormat);
            Record.LastModifiedAt = QDateTime::fromString(Query.value("LastModifiedAt").toString(), NormalTimeFormat);
            Record.Position = PointFromString(Query.value("Position").toString());
            Record.FoldConnectionCount = Query.value("FoldConnectionCount").toString().toInt();
            Record.Collapsed = BoolFromString(Query.value("Collapsed").toString());
            Record.ParentPosWhenCollapse = PointFromString(Query.value("ParentPosWhenCollapse").toString());
            Record.GroupItemID = Query.value("GroupItemID").toLongLong();
        }

        void SelectTextItems(const QSqlDatabase& DB, QList<TextItemRecord>& Result, const QVariant& GroupID = QVariant()){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM BaseItem NATURAL JOIN TextItem WHERE GroupItemID %1")
                    .arg(GroupIDToString(GroupID)), DB);
            Result.clear();
            TextItemRecord Record;
            while(Query.next()){
                ReadBaseInfo(Record, Query);
                Record.Content = Query.value("Content").toString();
                Result.append(Record);
            }
        }

        void SelectImageItems(const QSqlDatabase& DB, QList<ImageItemRecord>& Result, const QVariant& GroupID = QVariant()){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM BaseItem NATURAL JOIN ImageItem WHERE GroupItemID %1")
                    .arg(GroupIDToString(GroupID)), DB);
            Result.clear();
            ImageItemRecord Record;
            while(Query.next()){
                ReadBaseInfo(Record, Query);
                Record.Content = Query.value("Content").toByteArray();
                Result.append(Record);
            }
        }

        void SelectConnections(const QSqlDatabase& DB, QList<ConnectionRecord>& Result, const QVariant& GroupID = QVariant()){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM Connection WHERE GroupItemID %1")
                    .arg(GroupIDToString(GroupID)), DB);
            Result.clear();
            ConnectionRecord Record;
            while(Query.next()){
                Record.Identity = Query.value("Identity").toLongLong();
                Record.FromID = Query.value("FromID").toLongLong();
                Record.ToID = Query.value("ToID").toLongLong();
                Record.Collapsed = BoolFromString(Query.value("Collapsed").toString());
                Record.SrcConnected = BoolFromString(Query.value("SrcConnected").toString());
                Record.DstConnected = BoolFromString(Query.value("DstConnected").toString());
                Record.GroupItemID = Query.value("GroupItemID").toLongLong();
                Result.append(Record);
            }
        }

        void SelectGroups(const QSqlDatabase& DB, QList<ItemGroupRecord>& Result){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM ItemGroup"), DB);
            Result.clear();
            ItemGroupRecord Record;
            while(Query.next()){
                Record.GroupItemID = Query.value("GroupItemID").toLongLong();
                Record.PosWhenGrouping = PointFromString(Query.value("PosWhenGrouping").toString());
                Result.append(Record);
            }
        }

        virtual void Deserialize(const QString& InputPath, KCanvasController* CanvasController) override{
            {
                QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE", ConnectionName);
                DB.setDatabaseName(InputPath);
                bool Result = true;
                Result = DB.open();
                if(!Result)qDebug()<<DB.lastError();
                QList<TextItemRecord>TextItems;
                QList<ImageItemRecord>ImageItems;
                QList<ConnectionRecord>Connections;
                QList<ItemGroupRecord>Groups;
                // not grouped
                SelectTextItems(DB, TextItems);
                SelectImageItems(DB, ImageItems);
                SelectConnections(DB, Connections);
                for(const auto& Item : TextItems){
                    CanvasController->AddItemContrller(Item.ToController());
                }
                for(const auto& Item : ImageItems){
                    CanvasController->AddItemContrller(Item.ToController());
                }
                auto ItemMap = CanvasController->CreateItemMap();
                for(const auto& Conn : Connections){
                    CanvasController->AddConnectionController(Conn.ToController(ItemMap));
                }


                // grouped
                SelectGroups(DB, Groups);
                for(const auto& Group : Groups){
                    auto GroupController = Group.ToController();
                    SelectTextItems(DB, TextItems, Group.GroupItemID);
                    SelectImageItems(DB, ImageItems, Group.GroupItemID);
                    SelectConnections(DB, Connections, Group.GroupItemID);
                    for(const auto& Item : TextItems){
                        auto Controller = Item.ToController();
                        ItemMap.insert(Controller->GetIdentity(), Controller);
                        GroupController->AddItem(Controller);
                    }
                    for(const auto& Item : ImageItems){
                        auto Controller = Item.ToController();
                        ItemMap.insert(Controller->GetIdentity(), Controller);
                        GroupController->AddItem(Controller);
                    }
                    for(const auto& Conn : Connections){
                        auto ConnController = Conn.ToController(ItemMap);
                        if(Conn.SrcConnected && Conn.DstConnected){
                            GroupController->AddConnection(ConnController);
                        }
                        else if(Conn.SrcConnected){
                            GroupController->AddOutConnection(ConnController);
                        }
                        else if(Conn.DstConnected){
                            GroupController->AddInConnection(ConnController);
                        }
                    }
                    auto GroupItem = ItemMap.find(Group.GroupItemID);
                    if(GroupItem != ItemMap.end()){
                        auto GroupItemController = GroupItem.value();
                        GroupController->SetGroupItem(GroupItemController);
                        GroupItemController->SetItemGroupController(GroupController);
                        connect(GroupItemController, &KItemController::DestroyedSignal,
                                GroupController, &KItemGroupController::Destory);
                        const auto& InConns = GroupItemController->GetInConnections();
                        const auto& OutConns = GroupItemController->GetOutConnections();
                        for(auto Conn : InConns){
                            connect(Conn, &KConnectionController::DestroyedSignal,
                                    GroupController, &KItemGroupController::OnInConnectionDestroyed);
                        }
                        for(auto Conn : OutConns){
                            connect(Conn, &KConnectionController::DestroyedSignal,
                                    GroupController, &KItemGroupController::OnOutConnectionDestroyed);
                        }
                    }
                }

                DB.close();
            }
            QSqlDatabase::removeDatabase(ConnectionName);
        }
    };
}
