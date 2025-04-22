#include "views/viewapihandler.h"
#include "views/viewtypes.h"
#include "views/viewdbrepo.h"


ViewApiHandler::ViewApiHandler(PiMqttClient *client) : TypedApiHandler(client)
{
    m_handlers.insert("read", &ViewApiHandler::read);
    m_handlers.insert("list", &ViewApiHandler::list);
    m_handlers.insert("create", &ViewApiHandler::create);
    m_handlers.insert("clone", &ViewApiHandler::clone);
    m_handlers.insert("update", &ViewApiHandler::update);
    m_handlers.insert("delete", &ViewApiHandler::remove);
}

ViewApiHandler::~ViewApiHandler() = default;

void ViewApiHandler::read(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    ViewDetail detail;
    error = ViewDbRepo::read(id, detail);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, detail);
}

void ViewApiHandler::list(const PiMqttMessage &msg)
{
    QVector<View> views;
    ApiError error = ViewDbRepo::list(views);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, views);
}

void ViewApiHandler::create(const PiMqttMessage &msg)
{
    ViewCreateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    ViewDetail view;
    view.path = request.path;
    view.name = request.name;
    view.description = request.description;
    view.facility = "";
    view.groups = "";
    view.content = request.content;
    view.type = request.type;
    view.themeId = request.themeId;
    view.content = request.content;

    ApiError error = ViewDbRepo::create(view);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, view);
}

void ViewApiHandler::clone(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    ViewCloneRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    ViewDetail view;
    error = ViewDbRepo::read(id, view);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    view.name = request.name;
    view.description = request.description;

    error = ViewDbRepo::create(view);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, view);
}


void ViewApiHandler::update(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    QString controllerId;
    error = getControllerIdStringFromTopic(msg.topic(), controllerId);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    ViewUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    ViewDetail view;
    error = ViewDbRepo::read(id, view);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    view.name = request.name;
    view.description = request.description;
    view.content = request.content;
    view.type = request.type;
    view.themeId = request.themeId;

    error = ViewDbRepo::update(view);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, view);
}

void ViewApiHandler::remove(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    ViewDetail view;
    error = ViewDbRepo::read(id, view);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = ViewDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, view);
}

