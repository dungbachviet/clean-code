#ifndef PISCADA_API_SERVER_DOCUMENT_TYPES_H
#define PISCADA_API_SERVER_DOCUMENT_TYPES_H

#include "apiserverjson.h"

#include <QString>

class DocumentDetail
{
public:
    QString id;
    QString parentId;
    QString name;
    QString description;
    QString type;
    QString extension;
    int level;
    int writeAccess;
    int readAccess;

};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentDetail, id, parentId, name, description, type, extension, level, writeAccess, readAccess)

struct DocumentCreateRequest
{
    QString name;
    QString description;
    QString parentId;
    QString type;
    QString extension;
    int writeAccess;
    int readAccess;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentCreateRequest, name, description, parentId, type, extension, writeAccess, readAccess)


struct DocumentUpdateRequest
{
    QString name;
    QString description;
    int writeAccess;
    int readAccess;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentUpdateRequest, name, description, writeAccess, readAccess)

struct FileCreateRequest
{
    QString name;
    QString description;
    QString parentId;
    QString type;
    QString extension;
    int writeAccess;
    int readAccess;
    QString fileContent;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileCreateRequest, name, description, parentId, type, extension, writeAccess, readAccess, fileContent)

struct FileResponse
{
    DocumentDetail document;
    QString fileContent;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileResponse, document, fileContent)

struct DocumentMoveRequest          
{
    QString id;
    QString parentId;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentMoveRequest, id, parentId)

struct RestoreRequest 
{
    QString id;
    QString parentId;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RestoreRequest, id, parentId)

struct FolderNode
{
    QString id;
    QString name;
    QString type;
    QVector<QSharedPointer<FolderNode>> subFolders;
    QVector<DocumentDetail> files;
};

struct SimpleDocument
{
    QString id;
    QString name;
    QString extension;
    QString type;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleDocument, id, name, extension, type)

struct FolderPlainNode
{
    QString id;
    QString name;
    QString type;
    QVector<FolderPlainNode> subFolders;
    QVector<SimpleDocument> files;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FolderPlainNode, id, name, type, subFolders, files)


#endif // PISCADA_API_SERVER_DOCUMENT_TYPES_H
