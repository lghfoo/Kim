#pragma once
#include"Serializer.hpp"
#include <QStack>
#include <QTextCodec>
namespace Kim {
    class KTextSerializer: public KSerializer{
    public:
        struct KSavedContext{
            int Depth = 0;
            bool NeedComma = false;
        };
        struct KContext : public KSavedContext{
            KSerializer* CurrentSerializer = nullptr;
            int CurrentProgress = 0;
            int TotalProgress = 1;
            // For Serialize
            QStack<KSavedContext>ContextStack;
            // For Deserialize
            int LineNumber = 1;
            int ColumnNumber = 1;
            QString KeyToken = "";
            QString ValueToken = "";
            QChar LastChar = '\0';
            bool IsReadingValue = false;
            bool IsReadingKey = false;
            bool LastCharIsPlainBackslash = false;
            KItemController* CurrentItemController = nullptr;
            KConnectionController* CurrentConnectionController = nullptr;
            enum KState {StartState,
                       CanvasState,
                       ItemArrayToReadState,
                       ItemArrayReadingState,
                       ItemState,
                       ConnectionArrayToReadState,
                       ConnectionArrayReadingState,
                       ConnectionState
                       };
            KState State = StartState;
            // Methods
            void Save(){
                ContextStack.push(KSavedContext{this->Depth, this->NeedComma});
            }

            void Restore(){
                if(!ContextStack.isEmpty()){
                    KSavedContext SavedContext = ContextStack.pop();
                    this->Depth = SavedContext.Depth;
                    this->NeedComma = SavedContext.NeedComma;
                }
            }

            void Reset(){
                this->Depth = 0;
                this->NeedComma = false;
                this->CurrentProgress = 0;
                this->TotalProgress = 1;
                this->CurrentSerializer = nullptr;
                this->CurrentItemController = nullptr;
                this->CurrentConnectionController = nullptr;
                this->LineNumber = 1;
                this->ColumnNumber = 1;
                this->KeyToken = "";
                this->ValueToken = "";
                this->LastChar = '\0';
                this->IsReadingValue = false;
                this->IsReadingKey = false;
                this->LastCharIsPlainBackslash = false;
                ContextStack.clear();
            }

            QString DetailError(const QChar& Char){
                return QString("[%1:%2:%3]")
                        .arg(LineNumber)
                        .arg(ColumnNumber)
                        .arg(Char);
            }

            KContext(){
                ContextStack.clear();
            }

            ~KContext(){
            }
        };

        struct SerializeHelper{
#define BEGIN_OBJECT \
    Context.Save();\
    int Depth = ++Context.Depth;\
    QString Prefix(Depth, '\t');\
    StreamOut<<"{\n"
#define END_OBJECT \
    Context.Restore();\
    StreamOut<<QString("%1}%2\n").arg(QString(Depth-1, '\t')).arg(Context.NeedComma? "," : "")
#define BEGIN_ARRAY \
    Context.Save();\
    int Depth = ++Context.Depth;\
    QString Prefix(Depth, '\t');\
    StreamOut<<"[\n"
#define END_ARRAY \
    Context.Restore();\
    StreamOut<<QString("%1]%2\n").arg(QString(Depth-1, '\t')).arg(Context.NeedComma? "," : "")
#define PROGRESS_INC \
Context.CurrentProgress++;\
if(Context.CurrentSerializer){\
    emit Context.CurrentSerializer->ProgressSignal(Context.CurrentProgress);\
}
            static void SerializeCanvas(KCanvasController* CanvasController, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                const QList<KItemController*>& Items = CanvasController->ItemControlleres;
                const QList<KConnectionController*>& Connections = CanvasController->ConnectionControlleres;
                Context.TotalProgress = Items.size() + Connections.size();
                if(Context.CurrentSerializer){
                    emit Context.CurrentSerializer->TotalProgressSignal(Context.TotalProgress);
                }
                Context.CurrentProgress = 0;
                StreamOut << Prefix << "items: ";
                Context.NeedComma = true;
                SerializeItems(Items, StreamOut, Context);

                StreamOut << Prefix << "connections: ";
                Context.NeedComma = false;
                SerializeConnections(Connections, StreamOut, Context);
                if(Context.CurrentSerializer){
                    emit Context.CurrentSerializer->FinishedSignal();
                }
                END_OBJECT;
            }

            static void SerializeItems(const QList<KItemController*>& Items, QTextStream& StreamOut, KContext& Context){
                BEGIN_ARRAY;

                for(int i = 0; i < Items.size(); i++){
                    StreamOut << Prefix;
                    if(i != Items.size() - 1)Context.NeedComma = true;
                    else Context.NeedComma = false;
                    SerializeItem(Items[i], StreamOut, Context);
                }

                END_ARRAY;
            }

            static void SerializeConnections(const QList<KConnectionController*>& Connections, QTextStream& StreamOut, KContext& Context){
                BEGIN_ARRAY;

                for(int i = 0; i < Connections.size(); i++){
                    StreamOut << Prefix;
                    if(i != Connections.size() - 1)Context.NeedComma = true;
                    else Context.NeedComma = false;
                    SerializeConnection(Connections[i], StreamOut, Context);
                }

                END_ARRAY;
            }

            static void SerializeItem(KItemController* Item, QTextStream& StreamOut, KContext& Context){
                if(Item && Item->GetView()){
                    int ItemType = Item->GetView()->type();
                    switch (ItemType) {
                    case KTextItemView::Type:
                        SerializeTextItem(Item, StreamOut, Context);
                        break;
                    case KImageItemView::Type:
                        SerializeImageItem(Item, StreamOut, Context);
                        break;
                    default:
                        qDebug()<<"warning: unkown item type";
                        break;
                    }
                }
                else{
                    qDebug()<<"warning: skip null item.";
                }
                PROGRESS_INC
            }

#define OUT(KEY, VALUE)\
StreamOut << Prefix << (KEY) << " : \"" << (VALUE) <<"\"\n"
            static void CommonOut(KItemController* Item,
                                  KItemView* ItemView,
                                  QTextStream& StreamOut,
                                  const QString& Prefix){
                OUT("Type", ItemView->GetTypeAsString());
                OUT("Identity", Item->Identity);
                OUT("Alias", Item->Alias);
                OUT("CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
                OUT("LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
                OUT("Position", QString("%1, %2").arg(ItemView->pos().x()).arg(ItemView->pos().y()));
                OUT("Content", ToEscapedString(ItemView->GetContent()));
            }
            static void SerializeConnection(KConnectionController* Connection, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                OUT("From", Connection->GetSrcItemController()->Identity);
                OUT("To", Connection->GetDstItemController()->Identity);

                END_OBJECT;
                PROGRESS_INC
            }

            static void SerializeTextItem(KItemController* Item, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                CommonOut(Item, Item->GetView(), StreamOut, Prefix);
//                KTextItemController* ItemController = static_cast<KTextItemController*>(Item);
//                KTextItemView* ItemView = static_cast<KTextItemView*>(Item->GetView());
//                OUT("Type", ItemView->GetTypeAsString());
//                OUT("Identity", Item->Identity);
//                OUT("Alias", Item->Alias);
//                OUT("CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
//                OUT("LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
//                OUT("Position", QString("%1, %2").arg(ItemView->pos().x()).arg(ItemView->pos().y()));
//                OUT("Content", ToEscapedString(ItemView->GetContent()));

                END_OBJECT;
            }

            static void SerializeImageItem(KItemController* Item, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                CommonOut(Item, Item->GetView(), StreamOut, Prefix);
//                KImageItemController* ItemController = static_cast<KImageItemController*>(Item);
//                KImageItemView* ItemView = static_cast<KImageItemView*>(Item->GetView());
//                OUT("Type", ItemView->GetTypeAsString());
//                OUT("Identity", Item->Identity);
//                OUT("Alias", Item->Alias);
//                OUT("CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
//                OUT("LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
//                OUT("Position", QString("%1, %2").arg(ItemView->pos().x()).arg(ItemView->pos().y()));
//                OUT("Content", ToEscapedString(ItemView->GetContent()));

                END_OBJECT;
            }
#undef OUT
#undef BEGIN_OBJECT
#undef END_OBJECT
#undef BEGIN_ARRAY
#undef END_ARRAY
        };

        struct DeserializeHelper{
            static bool IsPlainChar(QChar Char){
                return ('a' <= Char && Char <= 'z')
                        || ('A' <= Char && Char <= 'Z')
                        || ('0' <= Char && Char <= '9')
                        || Char == '_';
            }
            static bool IsWhiteChar(QChar Char){
                return Char == '\t' || Char == '\n' || Char == ' ';
            }
            static void DeserializeCanvas(QTextStream& StreamIn, KCanvasController* CanvasController, KContext& Context){
                Context.Reset();
                QChar Char;
                while(!StreamIn.atEnd()){
                    StreamIn >> Char;
                    if(Context.IsReadingValue){
                        // '\' and '"' is special in string value
                        if(Char != '\\' && Char != '"'){
                            if(KTextItemView::IsInvalidChar(Char)){
                                // replace invalid char to ' '
                                //Context.ValueToken += ' ';

                                // skip invalid char
                            }
                            else{
                                Context.ValueToken += Char;
                            }
                        }
                        else if(Char == '"'){
                            // ....["] or ....\\["]
                            // end reading string value
                            if(Context.LastChar != '\\' ||Context.LastCharIsPlainBackslash){
                                Context.IsReadingValue = false;
                            }
                            // ....\["]
                            else{
                                Context.ValueToken += Char;
                            }
                        }
                        else if(Char == '\\'){
                            if(Context.LastChar == '\\'){
                                // this '\' is escape backslash
                                // ....\\[\]....
                                if(Context.LastCharIsPlainBackslash){
                                    Context.LastCharIsPlainBackslash = false;

                                }
                                // this '\' is plain backslash
                                // ....\[\]....
                                else{
                                    Context.ValueToken += Char;
                                    Context.LastCharIsPlainBackslash = true;
                                }

                            }
                            else{
                                // this '\' is escape backslash
                                // ....[\]....
                                Context.LastCharIsPlainBackslash = false;
                            }
                        }
                    }
                    else if(Context.IsReadingKey){
                        if(IsPlainChar(Char)){
                            Context.KeyToken += Char;
                        }
                        else{
                            Context.IsReadingKey = false;
                        }
                    }
                    else{
                        if(Char == '\t' || Char == ' '){
                            // pass
                        }
                        else if(Char == '{'){

                        }
                        else if(Char == '}'){

                        }
                        else if(Char == '['){

                        }
                        else if(Char == ']'){

                        }
                        else if(Char == '\n'){

                        }
                        else if(Char == '"'){
                            // ....["] or ....\\["]
                            // begin reading string value
                            Context.IsReadingValue = true;
                            Context.ValueToken = "";
                        }
                        else if(Char == '\\'){
                        }
                        else if(Char == ':'){

                        }
                        else if(IsPlainChar(Char)){
                            Context.KeyToken = Char;
                            Context.IsReadingKey = true;
                        }
                    }

                    switch (Context.State) {
                    case KContext::StartState:{
                        if(IsWhiteChar(Char)){
                            //pass
                        }
                        else if(Char == '{'){
                            Context.State = KContext::CanvasState;
                        }
                        else{
                            // error
                            qDebug()<<"error: start state error"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    case KContext::CanvasState:{
                        if(IsWhiteChar(Char) || Char == ',' || IsPlainChar(Char)){
                            //pass
                        }
                        else if(Char == '}'){
                            Context.State = KContext::StartState;
                        }
                        else if(Char == ':'){
                            if(Context.KeyToken.isEmpty()){
                                // error
                                qDebug()<<"error: canvas state error, expect kety token"<<Context.DetailError(Char);
                            }
                            else{
                                if(Context.KeyToken == "items"){
                                    Context.State = KContext::ItemArrayToReadState;
                                }
                                else if(Context.KeyToken == "connections"){
                                    Context.State = KContext::ConnectionArrayToReadState;
                                }
                                Context.KeyToken = "";
                            }
                        }
                        else{
                            // error
                            qDebug()<<"error: canvas state error"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    //////////////////////////////// Items ////////////////////////////////
                    case KContext::ItemArrayToReadState:{
                        if(IsWhiteChar(Char)){
                            // pass
                        }
                        else if(Char == '['){
                            Context.State = KContext::ItemArrayReadingState;
                        }
                        else{
                            // error
                            qDebug()<<"error: item array state"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    case KContext::ItemArrayReadingState:{
                        if(IsWhiteChar(Char) || Char == ','){
                            // pass
                        }
                        else if(Char == '{'){
                            Context.State = KContext::ItemState;
                        }
                        else if(Char == ']'){
                            Context.State = KContext::CanvasState;
                        }
                        else{
                            // error
                            qDebug()<<"error: item array reading state"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    case KContext::ItemState:{
                        if(IsWhiteChar(Char) || Context.IsReadingValue || Char == ',' || Char == ':' || IsPlainChar(Char)){

                        }
                        else if(Char == '"'){
                            if(Context.ValueToken.isEmpty()){
                                // pass
//                                qDebug()<<"warning: value token is empty"<<Context.DetailError(Char);
                            }
                            if(!Context.KeyToken.isEmpty()){
                                if(Context.CurrentItemController){
                                    if(Context.KeyToken == "Identity"){
                                        Context.CurrentItemController->Identity = Context.ValueToken;
                                    }
                                    else if(Context.KeyToken == "Alias"){
                                        Context.CurrentItemController->Alias = Context.ValueToken;
                                    }
                                    else if(Context.KeyToken == "CreatedAt"){
                                        Context.CurrentItemController->CreatedTime = QDateTime::fromString(Context.ValueToken, NormalTimeFormat);
                                    }
                                    else if(Context.KeyToken == "LastModifiedAt"){
                                        Context.CurrentItemController->LastModifiedTime = QDateTime::fromString(Context.ValueToken, NormalTimeFormat);
                                    }
                                    else if(Context.KeyToken == "Content"){
                                        auto ItemView = Context.CurrentItemController->GetView();
                                        ItemView->SetContent(Context.ValueToken);
//                                        auto Graphics = Context.CurrentItemController->GetView();
//                                        switch (Graphics->type()) {
//                                        case KTextItemView::Type:
//                                            qgraphicsitem_cast<KTextItemView*>(Graphics)
//                                                    ->SetText(Context.ValueToken);
//                                            break;
//                                        default:
//                                            qDebug()<<"error: unkown item type to set content"<<Context.DetailError(Char);
//                                            break;
//                                        }
                                    }
                                    else if(Context.KeyToken == "Position"){
                                        QStringList StrList = Context.ValueToken.split(',');
                                        if(StrList.size() != 2){
                                            qDebug()<<"error: position size error"<<Context.DetailError(Char);
                                        }
                                        else{
                                            qreal X = StrList[0].trimmed().toDouble();
                                            qreal Y = StrList[1].trimmed().toDouble();
                                            Context.CurrentItemController
                                                    ->GetView()->setPos(X, Y);
                                        }
                                    }
                                }
                                else{
                                    if(Context.KeyToken == "Type"){
//                                        qInfo()<<"info: creating controller of type: " + Context.ValueToken;
                                        if(Context.ValueToken == "TextItem"){
                                            Context.CurrentItemController = CanvasController
                                                    ->CreateAndAddItemController(KTextItemView::Type);
                                        }
                                        else if(Context.ValueToken == "ImageItem"){
                                            Context.CurrentItemController = CanvasController
                                                    ->CreateAndAddItemController(KImageItemView::Type);
                                        }
                                        else{
                                            qDebug()<<"error: unkown item type token"<<Context.DetailError(Char);
                                        }
                                    }
                                    else{
                                        // warning
                                        qDebug()<<QString("error: meet attr \"%1\" when item is not created").arg(Context.KeyToken)
                                               <<Context.DetailError(Char);
                                    }
                                }
                                Context.ValueToken = "";
                                Context.KeyToken = "";
                            }
                            else{
                                // pass
                            }
                        }
                        else if(Char == '}'){
                            Context.CurrentItemController = nullptr;
                            Context.State = KContext::ItemArrayReadingState;
                        }
                        else{
                            // error
                            qDebug()<<"error: item state"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    //////////////////////////////// Connections ////////////////////////////////
                    case KContext::ConnectionArrayToReadState:{
                        if(IsWhiteChar(Char)){
                            // pass
                        }
                        else if(Char == '['){
                            Context.State = KContext::ConnectionArrayReadingState;
                        }
                        else{
                            // error
                            qDebug()<<"error: connection array state"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    case KContext::ConnectionArrayReadingState:{
                        if(IsWhiteChar(Char) || Char == ','){
                            // pass
                        }
                        else if(Char == '{'){
                            Context.CurrentConnectionController =
                                    CanvasController->CreateAndAddConnectionController();
                            Context.State = KContext::ConnectionState;
                        }
                        else if(Char == ']'){
                            Context.State = KContext::CanvasState;
                        }
                        else{
                            // error
                            qDebug()<<"error: connection array reading state"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    case KContext::ConnectionState:{
                        if(IsWhiteChar(Char) || Context.IsReadingValue || Char == ',' || Char == ':' || IsPlainChar(Char)){
                            // pass
                        }
                        else if(Char == '"'){
                            if(Context.ValueToken.isEmpty()){
                                // pass
                            }
                            else if(!Context.ValueToken.isEmpty()){
                                auto ItemController = CanvasController
                                        ->GetItemByIdentity(Context.ValueToken);
                                if(!ItemController){
                                    qWarning()<<"warning: can not find item controller: " + Context.ValueToken;
                                }
                                else{
                                    if(Context.KeyToken == "From"){
                                        Context.CurrentConnectionController
                                                ->SetSrcItemController(ItemController);
                                    }
                                    else if(Context.KeyToken == "To"){
                                        Context.CurrentConnectionController
                                                ->SetDstItemController(ItemController);
                                    }
                                }
                            }
                            else{
                                // pass
                            }
                        }
                        else if(Char == '}'){
                            Context.CurrentConnectionController = nullptr;
                            Context.State = KContext::ConnectionArrayReadingState;
                        }
                        else{
                            // error
                            qDebug()<<"error: connection state"<<Context.DetailError(Char);
                        }
                        break;
                    }
                    }

                    Context.LastChar = Char;
                    if(Char == '\n'){
                        Context.LineNumber++;
                        Context.ColumnNumber = 1;
                    }
                    else{
                        Context.ColumnNumber++;
                    }
                }
            }
        };

    private:
        KContext Context;
    public:
        virtual void Serialize(KCanvasController* CanvasController, const QString& OutputPath) override{
            Context.Reset();
            Context.CurrentSerializer = this;
            QFile OutputFile(OutputPath);
            if (OutputFile.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream OutStream(&OutputFile);
                OutStream.setCodec(QTextCodec::codecForName("utf-8"));
                SerializeHelper::SerializeCanvas(CanvasController, OutStream, Context);
                OutputFile.close();
            }
            else{
                qDebug()<<"open file fail.";
            }
        }

        virtual void Deserialize(const QString& InputPath, KCanvasController* CanvasController) override{
            Context.Reset();
            Context.CurrentSerializer = this;
            QFile InputFile(InputPath);
            if(InputFile.open(QFile::ReadOnly)){
                QTextStream InStream(&InputFile);
                InStream.setCodec(QTextCodec::codecForName("utf-8"));
                DeserializeHelper::DeserializeCanvas(InStream, CanvasController, Context);
                InputFile.close();
            }
            else{
                qDebug()<<QString("open file \"%1\" fail.").arg(InputPath);
            }
        }

        static QString ToEscapedString(const QString& String){
            QString Ret = String;
            // replace '\' to '\\'
            Ret.replace("\\", "\\\\");
            // replace '"' to '\"'
            Ret.replace("\"", "\\\"");
            return Ret;
        }

        static QString FromEscapedString(const QString& String){
            QString Ret = String;
            // replace '\"' to '"'
            Ret.replace("\\\"", "\"");
            // replace '\\' to '\'
            Ret.replace("\\\\", "\\");
            return Ret;
        }
    };
}
