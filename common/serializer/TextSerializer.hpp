#pragma once
#include"Serializer.hpp"

#include <QTextCodec>
namespace Kim {
    class KTextSerializer: public KSerializer{
    public:
        struct KContext{
            // For Serialize
            int Depth = 0;
            bool NeedComma = false;
            KContext* SavedContext = nullptr;
            // For Deserialize
            QString KeyToken = "";
            QString ValueToken = "";
            QChar LastChar = '\0';
            QChar LastNonEmptyChar = '\0';
            bool IsReadingValue = false;
            bool IsReadingKey = false;
            bool LastCharIsPlainBackslash = false;
            // Methods
            void Save(){
                if(!SavedContext){
                    SavedContext = new KContext{this->Depth, this->NeedComma};
                }
                else{
                    SavedContext = new (SavedContext)KContext{this->Depth, this->NeedComma};
                }

            }

            void Restore(){
                if(SavedContext){
                    this->Depth = SavedContext->Depth;
                    this->NeedComma = SavedContext->NeedComma;
                }
            }

            void Reset(){
                this->Depth = 0;
                this->NeedComma = false;
            }

            ~KContext(){
                if(SavedContext)
                    delete SavedContext;
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
            static void SerializeCanvas(KCanvasController* CanvasController, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                StreamOut << Prefix << "items: ";
                const QList<KItemController*>& Items = CanvasController->ItemViewControlleres;
                SerializeItems(Items, StreamOut, Context);

                StreamOut << Prefix << "connections: ";
                const QList<KConnectionController*>& Connections = CanvasController->ConnectionControlleres;
                SerializeConnections(Connections, StreamOut, Context);

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
                    int ItemType = Item->GetView()->ToGraphics()->type();
                    switch (ItemType) {
                    case KTextItemView::Type:
                        SerializeTextItem(Item, StreamOut, Context);
                        break;
                    default:
                        qDebug()<<"warning: unkown item type";
                        break;
                    }
                }
                else{
                    qDebug()<<"warning: skip null item.";
                }
            }

#define OUT(KEY, VALUE)\
StreamOut << Prefix << (KEY) << " : \"" << (VALUE) <<"\"\n"
            static void SerializeConnection(KConnectionController* Connection, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                OUT("From", Connection->GetSrcItemController()->Identity);
                OUT("To", Connection->GetDstItemController()->Identity);

                END_OBJECT;
            }

            static void SerializeTextItem(KItemController* Item, QTextStream& StreamOut, KContext& Context){
                BEGIN_OBJECT;

                KTextItemController* ItemController = dynamic_cast<KTextItemController*>(Item);
                KTextItemView* ItemView = dynamic_cast<KTextItemView*>(Item->GetView());
                if(!ItemController || !ItemView){
                    qDebug()<<"error: type mismatched, KTextItemView is expected.";
                    return;
                }
                OUT("Type", ItemView->GetTypeAsString());
                OUT("Identity", Item->Identity);
                OUT("Alias", Item->Alias);
                OUT("CreatedAt", Item->CreatedTime.toString(NormalTimeFormat));
                OUT("LastModifiedAt", Item->LastModifiedTime.toString(NormalTimeFormat));
                OUT("Content", ToEscapedString(ItemView->GetText()));
                OUT("Position", QString("%1, %2").arg(ItemView->pos().x()).arg(ItemView->pos().y()));

                END_OBJECT;
            }
#undef OUT(KEY, VALUE)

#undef BEGIN_OBJECT
#undef END_OBJECT
#undef BEGIN_ARRAY
#undef END_ARRAY
        };

        struct DeserializeHelper{
            static bool IsPlainChar(QChar Char){
                return ('a' <= Char && Char <= 'z')
                        || ('A' <= Char && Char <= 'Z')
                        || Char == '_';
            }
            static void DeserializeCanvas(QTextStream& StreamIn, KCanvasController* CanvasController, KContext& Context){
                Context.Reset();
                QChar Char;
                while(!StreamIn.atEnd()){
                    StreamIn >> Char;
                    if(Context.IsReadingValue){
                        // '\' and '"' is special in string value
                        if(Char != '\\' && Char != '"'){
                            Context.ValueToken += Char;
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
                            if(Context.LastChar != '\\' || Context.LastCharIsPlainBackslash){
                                Context.IsReadingValue = true;
                            }
                            // ....\["]
                            else{
                                // pass
                            }
                        }
                        else if(Char == '\\'){
                        }
                        else if(Char == ':'){

                        }
                        else if(!Context.IsReadingValue && IsPlainChar(Char)){
                            if(IsPlainChar(Context.LastChar)){
                                Context.KeyToken += Char;
                            }
                            else{
                                Context.KeyToken = Char;
                                Context.IsReadingKey = true;
                            }
                        }
                    }

                    if(Context.IsReadingKey && !IsPlainChar(Char)){
                        Context.IsReadingKey = false;
                    }
                    Context.LastChar = Char;

                }
            }
        };

    private:
        KContext Context;
    public:
        virtual void Serialize(KCanvasController* CanvasController, const QString& OutputPath) override{
            Context.Reset();
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
            QFile InputFile(InputPath);
            if(InputFile.open(QFile::ReadOnly)){
                QTextStream InStream(&InputFile);
                InStream.setCodec(QTextCodec::codecForName("utf-8"));
                DeserializeHelper::DeserializeCanvas(InStream, CanvasController, Context);
                InputFile.close();
            }
            else{
                qDebug()<<"open file fail.";
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
            // replace '\\' to '\'
            Ret.replace("\\\"", "\"");
            // replace '\"' to '"'
            Ret.replace("\\\\", "\\");
            return Ret;
        }
    };
}
