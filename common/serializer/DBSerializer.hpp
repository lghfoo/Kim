#pragma once
#include<QtSql>
#include"Serializer.hpp"
namespace Kim {
    class KDBSerializer : public KSerializer{
    private:
        bool NeedRecreateTable = true;
        bool UseTransaction = true;
        bool EmitFinishedSignal = true;
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

        QString CanvasIDToString(const QVariant& CanvasID){
            return QString::number(CanvasID.toLongLong());
        }

        ////////////////////////////////////////////////////////////////
        /// Serialize
        ////////////////////////////////////////////////////////////////
        void RecreateTable(QSqlQuery& Query){
            bool Result = true;
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
            CheckResult(Result, Query);
            Result = Query.exec("DROP TABLE IF EXISTS 'Canvas'");
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
                                "'GroupItemID'          INTEGER,"
                                "'CanvasID'             INTEGER,"
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
                                "'GroupItemID'  INTEGER,"
                                "'CanvasID'     INTEGER,"
                                "PRIMARY KEY('Identity')"
                                ")");
            CheckResult(Result, Query);
            Result = Query.exec("CREATE TABLE 'ItemGroup' ("
                                "'GroupItemID'  INTEGER,"
                                "'PosWhenGrouping'  TEXT,"
                                "'CanvasID'     INTEGER,"
                                "PRIMARY KEY('GroupItemID')"
                                ")");
            CheckResult(Result, Query);
            Result = Query.exec("CREATE TABLE 'Canvas' ("
                                "'CanvasID'     INTEGER,"
                                "'CanvasName'   TEXT,"
                                "PRIMARY KEY('CanvasID')"
                                ")");
            CheckResult(Result, Query);
        }
        void InsertBaseItem(KItemController* Item, QSqlQuery& Query,
                            const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            Query.bindValue(":Identity", Item->Identity);
            Query.bindValue(":Alias", Item->Alias);
            Query.bindValue(":CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
            Query.bindValue(":LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
            Query.bindValue(":Position", PointToString(Item->GetView()->pos()));
            Query.bindValue(":FoldConnectionCount", Item->FoldConnectionCount);
            Query.bindValue(":Collapsed", BoolToChar(Item->Collapsed));
            Query.bindValue(":ParentPosWhenCollapse", PointToString(Item->ParentPosWhenCollapse));
            Query.bindValue(":GroupItemID", GroupID);
            Query.bindValue(":CanvasID", CanvasID);
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

        void InsertConnection(KConnectionController* Conn, QSqlQuery& Query,
                              const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            Query.bindValue(":Identity", QVariant(Conn->GetIdentity()));
            Query.bindValue(":FromID", Conn->GetSrcItemController()->Identity);
            Query.bindValue(":ToID", Conn->GetDstItemController()->Identity);
            Query.bindValue(":Collapsed", BoolToChar(Conn->IsCollapsed()));
            Query.bindValue(":SrcConnected", BoolToChar(Conn->IsSrcConnected()));
            Query.bindValue(":DstConnected", BoolToChar(Conn->IsDstConnected()));
            Query.bindValue(":GroupItemID", GroupID);
            Query.bindValue(":CanvasID", CanvasID);
            bool Result = Query.exec();
            CheckResult(Result, Query);
        }

        void InsertItems(const QLinkedList<KItemController*>& Items, QSqlQuery& Query,
                         const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            QString BaseItemPrepare = QString("INSERT INTO BaseItem VALUES (:Identity, :Alias, :CreatedAt,"
                                      ":LastModifiedAt, :Position, :FoldConnectionCount, :Collapsed,"
                                      ":ParentPosWhenCollapse, :GroupItemID, :CanvasID)");
            QString TextItemPrepare = "INSERT INTO TextItem VALUES (:Identity, :Content)";
            QString ImageItemPrepare = "INSERT INTO ImageItem VALUES (:Identity, :Content)";
            Query.prepare(BaseItemPrepare);
            for(auto Item : Items){
                InsertBaseItem(Item, Query, CanvasID, GroupID);
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

        void InsertConnections(const QLinkedList<KConnectionController*>& Conns, QSqlQuery& Query,
                               const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            QString ConnectionPrepare = QString("INSERT INTO Connection VALUES (:Identity, :FromID, :ToID, :Collapsed, "
                                        ":SrcConnected, :DstConnected, :GroupItemID, :CanvasID)");
            Query.prepare(ConnectionPrepare);
            for(auto Conn : Conns){
                InsertConnection(Conn, Query, CanvasID, GroupID);
            }
        }

        void InsertGroups(const QList<KItemGroupController*>& Groups, QSqlQuery& Query,
                          const QVariant& CanvasID){
            QString GroupPrepare = QString("INSERT INTO ItemGroup VALUES (:GroupItemID, :PosWhenGrouping, :CanvasID)");
            Query.prepare(GroupPrepare);
            for(auto Group : Groups){
                Query.bindValue(":GroupItemID", Group->GetGroupID());
                Query.bindValue(":PosWhenGrouping", PointToString(Group->GetPosWhenGrouping()));
                Query.bindValue(":CanvasID", CanvasID);
                bool Result = Query.exec();
                CheckResult(Result, Query);
            }
        }

        void InsertCanvases(const QList<KCanvasController*>& Canvases, QSqlQuery& Query){
            QString CanvasPrepare = QString("INSERT INTO Canvas VALUES (:CanvasID, :CanvasName)");
            Query.prepare(CanvasPrepare);
            for(auto Canvas : Canvases){
                Query.bindValue(":CanvasID", Canvas->GetCanvasID());
                Query.bindValue(":CanvasName", Canvas->GetCanvasName());
                bool Result = Query.exec();
                CheckResult(Result, Query);
            }
        }

        ////////////////////////////////////////////////////////////////
        /// Deserialize
        ////////////////////////////////////////////////////////////////
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
            qint64 CanvasID;
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
            qint64 CanvasID;
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
            qint64 CanvasID;
            KItemGroupController* ToController()const{
                auto Controller = new KItemGroupController;
                Controller->SetPosWhenGroupping(PosWhenGrouping);
                return Controller;
            }
        };

        struct CanvasRecord{
            qint64 CanvasID;
            QString CanvasName;
            KCanvasController* ToController()const{
                auto Controller = new KCanvasController;
                Controller->SetCanvasID(CanvasID);
                Controller->SetCanvasName(CanvasName);
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
            Record.CanvasID = Query.value("CanvasID").toLongLong();
        }

        void SelectTextItems(const QSqlDatabase& DB, QList<TextItemRecord>& Result, const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM BaseItem NATURAL JOIN TextItem WHERE GroupItemID %1 and CanvasID=%2")
                    .arg(GroupIDToString(GroupID))
                    .arg(CanvasID.toLongLong()), DB);
            Result.clear();
            TextItemRecord Record;
            while(Query.next()){
                ReadBaseInfo(Record, Query);
                Record.Content = Query.value("Content").toString();
                Result.append(Record);
            }
        }

        void SelectImageItems(const QSqlDatabase& DB, QList<ImageItemRecord>& Result, const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM BaseItem NATURAL JOIN ImageItem WHERE GroupItemID %1 and CanvasID=%2")
                                        .arg(GroupIDToString(GroupID))
                                        .arg(CanvasID.toLongLong()), DB);
            Result.clear();
            ImageItemRecord Record;
            while(Query.next()){
                ReadBaseInfo(Record, Query);
                Record.Content = Query.value("Content").toByteArray();
                Result.append(Record);
            }
        }

        void SelectConnections(const QSqlDatabase& DB, QList<ConnectionRecord>& Result, const QVariant& CanvasID, const QVariant& GroupID = QVariant()){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM Connection WHERE GroupItemID %1 and CanvasID=%2")
                                        .arg(GroupIDToString(GroupID))
                                        .arg(CanvasID.toLongLong()), DB);
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
                Record.CanvasID = Query.value("CanvasID").toLongLong();
                Result.append(Record);
            }
        }

        void SelectGroups(const QSqlDatabase& DB, QList<ItemGroupRecord>& Result, const QVariant& CanvasID){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM ItemGroup WHERE CanvasID=%1").arg(CanvasID.toLongLong()), DB);
            Result.clear();
            ItemGroupRecord Record;
            while(Query.next()){
                Record.GroupItemID = Query.value("GroupItemID").toLongLong();
                Record.PosWhenGrouping = PointFromString(Query.value("PosWhenGrouping").toString());
                Record.CanvasID = Query.value("CanvasID").toLongLong();
                Result.append(Record);
            }
        }

        void SelectCanvases(const QSqlDatabase& DB, QList<CanvasRecord>& Result){
            QSqlQuery Query = QSqlQuery(QString("SELECT * FROM Canvas"), DB);
            Result.clear();
            CanvasRecord Record;
            while(Query.next()){
                Record.CanvasID = Query.value("CanvasID").toLongLong();
                Record.CanvasName = Query.value("CanvasName").toString();
                Result.append(Record);
            }
        }
    public:
        KDBSerializer(const QString& DatabaseName): KSerializer(DatabaseName){
            QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE", ConnectionName);
            DB.setDatabaseName(DatabaseName);
            DB.open();

        }
        virtual ~KDBSerializer()override{
            {
                QSqlDatabase DB = QSqlDatabase::database(ConnectionName);
                DB.close();
            }
            QSqlDatabase::removeDatabase(ConnectionName);
        }
        /**
         * @brief Serialize
         * @param CanvasController: the controller to serialize from, should be not null
         * @param OutputPath: output path
         */
        virtual void Serialize(KCanvasController* CanvasController) override{
            {
                QSqlDatabase DB = QSqlDatabase::database(ConnectionName);
                bool TranscationResult = true;
                if(UseTransaction){
                    TranscationResult = DB.transaction();
                    if(!TranscationResult)
                        qDebug()<<DB.lastError();
                }

                QSqlQuery Query(DB);
                if(NeedRecreateTable){
                    RecreateTable(Query);
                }
                // Insert Data
                const auto& Items = CanvasController->ItemControlleres;
                const auto& Conns = CanvasController->ConnectionControlleres;
                const auto& CanvasID = CanvasController->CanvasID;
                // insert items
                InsertItems(Items, Query, CanvasID);
                // insert connections
                InsertConnections(Conns, Query, CanvasID);
                // insert groups
                const auto& Groups = CanvasController->GroupControlleres.values();
                for(auto Group : Groups){
                    InsertItems(Group->GetItems(), Query, CanvasID, Group->GetGroupID());
                    InsertConnections(Group->GetAllConnections(), Query, CanvasID, Group->GetGroupID());
                }
                InsertGroups(Groups, Query, CanvasID);
                // insert canvases
                InsertCanvases({CanvasController}, Query);
                if(UseTransaction && TranscationResult)
                    if(!DB.commit())
                        qDebug()<<DB.lastError();
            }
            if(EmitFinishedSignal)
                emit FinishedSignal();
        }
        virtual void Serialize(KMainViewController* MainController) override{
            {
                auto DB = QSqlDatabase::database(ConnectionName);
                QSqlQuery Query(DB);
                DB.transaction();
                RecreateTable(Query);

                this->UseTransaction = false;
                this->NeedRecreateTable = false;
                this->EmitFinishedSignal = false;

                const auto& Canvases = MainController->GetCanvasControlleres();
                for(auto Canvas : Canvases){
                    this->Serialize(Canvas);
                }

                DB.commit();
                emit FinishedSignal();
            }
        }
        /**
         * @brief Deserialize
         * @param InputPath: input path
         * @param CanvasController: the controller to serialize to, should be not null
         */
        virtual void Deserialize(KCanvasController* CanvasController, const QVariant& OriCanvasID = QVariant()) override{
            CanvasController->Clear();
            {
                QSqlDatabase DB = QSqlDatabase::database(ConnectionName);
                QList<TextItemRecord>TextItems;
                QList<ImageItemRecord>ImageItems;
                QList<ConnectionRecord>Connections;
                QList<ItemGroupRecord>Groups;
                QVariant CanvasID = OriCanvasID;
                // 如果没有canvas id，那么就是加载canvas
                // 而不是在open project
                if(CanvasID.isNull()){
                    QList<CanvasRecord> Canvases;
                    SelectCanvases(DB, Canvases);
                    if(Canvases.isEmpty())return;
                    CanvasID = Canvases.front().CanvasID;
                    CanvasController->SetCanvasID(CanvasID.toLongLong());
                }
                // not grouped
                SelectTextItems(DB, TextItems, CanvasID);
                SelectImageItems(DB, ImageItems, CanvasID);
                SelectConnections(DB, Connections, CanvasID);
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
                SelectGroups(DB, Groups, CanvasID);
                for(const auto& Group : Groups){
                    auto GroupController = Group.ToController();
                    SelectTextItems(DB, TextItems, CanvasID, Group.GroupItemID);
                    SelectImageItems(DB, ImageItems, CanvasID, Group.GroupItemID);
                    SelectConnections(DB, Connections, CanvasID, Group.GroupItemID);
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
            }
        }
        virtual void Deserialize(KMainViewController* MainController) override{
            MainController->Clear();
            {
                auto DB = QSqlDatabase::database(ConnectionName);
                QList<CanvasRecord> Canvases;
                SelectCanvases(DB, Canvases);
                for(const auto& Canvas : Canvases){
                    auto CanvasWrapper = MainController->CreateCanvasWrapper();
                    auto CanvasController = CanvasWrapper->GetCanvasController();
                    this->Deserialize(CanvasController, Canvas.CanvasID);
                    CanvasController->SetCanvasID(Canvas.CanvasID);
                    CanvasController->SetCanvasName(Canvas.CanvasName);
                    MainController->AddCanvasWrapper(CanvasWrapper);
                }
            }
        }
    };
}
