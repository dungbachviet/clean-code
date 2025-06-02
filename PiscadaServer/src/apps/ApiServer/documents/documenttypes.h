#ifndef PISCADA_API_SERVER_DOCUMENT_TYPES_H
#define PISCADA_API_SERVER_DOCUMENT_TYPES_H

#include "apiserverjson.h"

#include <QString>

class DocumentDetail 
{
public:
    QString id;
    QString name;
    QString extension;
    QString type;
    QString parentId;
    QString description;
    int writeAccess;
    int readAccess;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentDetail, id, name, extension, type, parentId, description, writeAccess, readAccess)

struct FolderCreateRequest
{
    QString name;
    QString description;
    QString parentId;
    QString type;
    QString extension;
    int writeAccess;
    int readAccess;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FolderCreateRequest, name, description, parentId, type, extension, writeAccess, readAccess)

struct DocumentUpdateRequest
{
    QString name;
    QString description;
    int writeAccess;
    int readAccess;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentUpdateRequest, name, description, writeAccess, readAccess)

struct DocumentMoveRequest          
{
    QString id;
    QString parentId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentMoveRequest, id, parentId)

struct DocumentRestoreRequest 
{
    QString id;
    QString parentId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentRestoreRequest, id, parentId)

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

struct FileStartRequest
{
    QString name;
    QString description;
    QString parentId;
    QString type;
    QString extension;
    int writeAccess;
    int readAccess;
    int totalChunks;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileStartRequest, name, description, parentId, type, extension, writeAccess, readAccess, totalChunks)

struct FileChunkRequest {
    QString id;
    QString name;
    QString extension;
    QString chunkBase64;
    int chunkIndex;
    int totalChunks;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileChunkRequest, id, name, extension, chunkBase64, chunkIndex, totalChunks)

struct FileFinishRequest
{
    QString id;
    QString name;
    QString description;
    QString parentId;
    QString type;
    QString extension;
    int writeAccess;
    int readAccess;
    int totalChunks;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileFinishRequest, id, name, description, parentId, type, extension, writeAccess, readAccess, totalChunks)

struct FileChunkResponse
{
    QString id;
    QString parentId;
    QString name;
    QString description;
    QString type;
    QString extension;
    int writeAccess;
    int readAccess;
    QString chunkBase64;
    int chunkIndex;
    int totalChunks;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FileChunkResponse, id, parentId, name, description, type, extension, writeAccess, readAccess, chunkBase64, chunkIndex, totalChunks)

struct FolderNode
{
    QString id;
    QString name;
    QString type;
    QVector<QSharedPointer<FolderNode>> subFolders;
    QVector<DocumentDetail> files;
};

struct FolderPlainNode
{
    QString id;
    QString name;
    QString type;
    QVector<FolderPlainNode> subFolders;
    QVector<DocumentDetail> files;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FolderPlainNode, id, name, type, subFolders, files)

#endif // PISCADA_API_SERVER_DOCUMENT_TYPES_H
