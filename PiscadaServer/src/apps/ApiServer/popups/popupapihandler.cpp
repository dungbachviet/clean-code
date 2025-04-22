#include "popups/popupapihandler.h"
#include "popups/popupdbrepo.h"
#include "popups/popuptypes.h"


PopupApiHandler::PopupApiHandler(PiMqttClient *client)
    : TypedApiHandler<PopupApiHandler>(client)
{
    m_handlers.insert("read", &PopupApiHandler::read);
    m_handlers.insert("list", &PopupApiHandler::list);
    m_handlers.insert("create", &PopupApiHandler::create);
    m_handlers.insert("clone", &PopupApiHandler::clone);
    m_handlers.insert("update", &PopupApiHandler::update);
    m_handlers.insert("delete", &PopupApiHandler::remove);
}

PopupApiHandler::~PopupApiHandler() = default;

void PopupApiHandler::read(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    PopupDetail popup;
    error = PopupDbRepo::read(id, popup);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, popup);
}

void PopupApiHandler::list(const PiMqttMessage &msg)
{
    QVector<Popup> popups;
    ApiError error = PopupDbRepo::list(popups);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, popups);
}

void PopupApiHandler::create(const PiMqttMessage &msg)
{
    PopupCreateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    PopupDetail popup;
    popup.name = request.name;
    popup.description = request.description;
    if (request.content.has_value())
    {
        popup.content = request.content.value();
    }
    else
    {
        popup.content = Json::object();
    }

    ApiError error = PopupDbRepo::create(popup);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, popup);
}

void PopupApiHandler::clone(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    PopupCloneRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    PopupDetail popup;
    error = PopupDbRepo::read(id, popup);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    popup.name = request.name;
    popup.description = request.description;

    error = PopupDbRepo::create(popup);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, popup);
}

void PopupApiHandler::update(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    PopupUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    PopupDetail popup;
    popup.id = id;
    popup.name = request.name;
    popup.description = request.description;
    popup.content = request.content;

    error = PopupDbRepo::update(popup);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, popup);
}

void PopupApiHandler::remove(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    PopupDetail popup;
    error = PopupDbRepo::read(id, popup);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = PopupDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, popup);
}

