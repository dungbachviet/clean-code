#include "documents/documentapihandler.h"
#include "documents/documenttypes.h"
#include "documents/documentdbrepo.h"

#include <QDebug>

DocumentApiHandler::DocumentApiHandler(PiMqttClient *client) : TypedApiHandler(client)
{
    m_handlers.insert("createFolder", &DocumentApiHandler::createFolder);
    m_handlers.insert("getFolder", &DocumentApiHandler::getFolder);
    m_handlers.insert("updateFolder", &DocumentApiHandler::updateFolder);
    m_handlers.insert("deleteFolder", &DocumentApiHandler::deleteFolder);
    m_handlers.insert("listAll", &DocumentApiHandler::listAll);
    m_handlers.insert("listChildren", &DocumentApiHandler::listChildren);
    m_handlers.insert("createFile", &DocumentApiHandler::createFile);
    m_handlers.insert("getFile", &DocumentApiHandler::getFile);
    m_handlers.insert("updateFile", &DocumentApiHandler::updateFile);
    m_handlers.insert("deleteFile", &DocumentApiHandler::deleteFile);
    m_handlers.insert("moveDocument", &DocumentApiHandler::moveDocument);
    m_handlers.insert("softDeleteFile", &DocumentApiHandler::softDeleteFile);   
    m_handlers.insert("restoreFile", &DocumentApiHandler::restoreFile);
    m_handlers.insert("listDeletedFiles", &DocumentApiHandler::listDeletedFiles);
    m_handlers.insert("getTree", &DocumentApiHandler::getTree);
    m_handlers.insert("getAllTree", &DocumentApiHandler::getAllTree);

}
DocumentApiHandler::~DocumentApiHandler() = default;

void DocumentApiHandler::createFolder(const PiMqttMessage &msg)
{
    DocumentCreateRequest request;

    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }
    if (DocumentDbRepo::isNameExists(request.name, request.parentId, request.type))
    {
        return sendErrorResponse(msg, ApiError::conflict("This name already exists in the parent folder"));
    }

    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    DocumentDetail document;
    document.name = request.name;
    document.description = request.description;
    document.parentId = request.parentId;
    document.type = request.type;
    document.extension = request.extension;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;
    document.level = DocumentDbRepo::getLevel(document.parentId);
    document.id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    ApiError error = DocumentDbRepo::create(document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::getFolder(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::updateFolder(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    document.name = request.name;
    document.description = request.description;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;

    error = DocumentDbRepo::update(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::deleteFolder(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = DocumentDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::listAll(const PiMqttMessage &msg)
{
    QVector<DocumentDetail> documents;
    ApiError error = DocumentDbRepo::list(documents);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, documents);
}

void DocumentApiHandler::listChildren(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    QVector<DocumentDetail> documents;
    error = DocumentDbRepo::listChildren(id, documents);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, documents);
}

void DocumentApiHandler::createFile(const PiMqttMessage &msg)
{
    FileCreateRequest request;

    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    if (DocumentDbRepo::isNameExists(request.name, request.parentId, request.type))
    {
        return sendErrorResponse(msg, ApiError::conflict("This name already exists in the parent folder"));
    }

    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    DocumentDetail document;
    document.name = request.name;
    document.description = request.description;
    document.parentId = request.parentId;
    document.type = request.type;
    document.extension = request.extension;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;
    document.level = DocumentDbRepo::getLevel(document.parentId);
    document.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QByteArray decodedContent;
    ApiError decodeError= DocumentDbRepo::decodeAndValidateBase64File(request.fileContent.toUtf8(), document.extension, decodedContent);
    if (decodeError.isError()) 
    { 
        return sendErrorResponse(msg, decodeError);
    }
    ApiError fileError = DocumentDbRepo::saveFileToSystem(document.id, document.extension, decodedContent);

    if (fileError.isError())
    {
        return sendErrorResponse(msg, fileError);
    }

    ApiError error = DocumentDbRepo::create(document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    sendResponse(msg, document);
}

void DocumentApiHandler::getFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    QString fileContentBase64;
    ApiError fileError = DocumentDbRepo::readFileFromSystem(document.id, document.extension, fileContentBase64);
    if (fileError.isError())
    {
        return sendErrorResponse(msg, fileError);
    }

    FileResponse response;
    response.document = document;
    response.fileContent = fileContentBase64;

    sendResponse(msg, response);
}

void DocumentApiHandler::updateFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    document.name = request.name;
    document.description = request.description;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;

    error = DocumentDbRepo::update(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::deleteFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = DocumentDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::moveDocument(const PiMqttMessage &msg) {

    DocumentMoveRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);

    if (jsonError.type() != JsonError::NoError) 
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    DocumentDetail document;
 
    ApiError error = DocumentDbRepo::read(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    if (DocumentDbRepo::isNameExists(document.name, request.parentId, document.type))
    {
        return sendErrorResponse(msg, ApiError::conflict("This name already exists in the parent folder"));
    }

    document.parentId = request.parentId;
    int oldLevel = document.level;
    int newLevel = DocumentDbRepo::getLevel(request.parentId);
    int deltaLevel = newLevel - oldLevel ;
    document.level = newLevel;

    error = DocumentDbRepo::move(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = DocumentDbRepo::updateLevelForDescendants(request.id, deltaLevel);
    if (error.isError()) 
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::softDeleteFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = DocumentDbRepo::softDelete(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    sendResponse(msg, document);

}

void DocumentApiHandler::restoreFile(const PiMqttMessage &msg)
{
    RestoreRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    DocumentDetail document;
    ApiError error = DocumentDbRepo::read(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    if (DocumentDbRepo::isNameExists(document.name, request.parentId, document.type))
    {
        return sendErrorResponse(msg, ApiError::conflict("This name already exists in the parent folder"));
    }

    document.parentId = request.parentId;
    document.level = DocumentDbRepo::getLevel(request.parentId);
    
    error = DocumentDbRepo::restore(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    sendResponse(msg, document);
}

void DocumentApiHandler::listDeletedFiles(const PiMqttMessage &msg)
{
    QVector<DocumentDetail> deletedFiles; 
    ApiError error = DocumentDbRepo::listDeleted(deletedFiles);
    if (error.isError()) 
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, deletedFiles);
}

void DocumentApiHandler::getTree(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    DocumentDetail rootFolder;
    error = DocumentDbRepo::read(id, rootFolder);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    if (rootFolder.type != "folder")
    {
        return sendErrorResponse(msg, ApiError::InvalidRequest);
    }

    QVector<DocumentDetail> flatList = DocumentDbRepo::getAllDescendants(id);
    
    QSharedPointer<FolderNode> rootTree = DocumentDbRepo::buildFolderTree(id, rootFolder, flatList);

    sendResponse(msg, DocumentDbRepo::convertToPlainNode(rootTree));
}

void DocumentApiHandler::getAllTree(const PiMqttMessage &msg)
{
    QVector<DocumentDetail> flatList;
    ApiError err = DocumentDbRepo::list(flatList);

    if (err.isError())
    {
        return sendErrorResponse(msg, err);
    }

    QSharedPointer<FolderNode> rootTree = DocumentDbRepo::buildAllTree(flatList);
    sendResponse(msg, DocumentDbRepo::convertToPlainNode(rootTree));
}