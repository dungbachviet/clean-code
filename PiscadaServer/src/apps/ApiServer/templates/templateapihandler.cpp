#include "templates/templateapihandler.h"
#include "templates/templatedbrepo.h"
#include "templates/templatetypes.h"


TemplateApiHandler::TemplateApiHandler(PiMqttClient *client)
    : TypedApiHandler<TemplateApiHandler>(client)
{
    m_handlers.insert("read", &TemplateApiHandler::read);
    m_handlers.insert("list", &TemplateApiHandler::list);
    m_handlers.insert("create", &TemplateApiHandler::create);
    m_handlers.insert("clone", &TemplateApiHandler::clone);
    m_handlers.insert("update", &TemplateApiHandler::update);
    m_handlers.insert("delete", &TemplateApiHandler::remove);
}

TemplateApiHandler::~TemplateApiHandler() = default;

void TemplateApiHandler::read(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    TemplateDetail tmpl;
    error = TemplateDbRepo::read(id, tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, tmpl);
}

void TemplateApiHandler::list(const PiMqttMessage &msg)
{
    QVector<Template> templates;
    ApiError error = TemplateDbRepo::list(templates);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, templates);
}

void TemplateApiHandler::create(const PiMqttMessage &msg)
{
    TemplateCreateRequest req;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &req);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    TemplateDetail tmpl;
    tmpl.name = req.name;
    tmpl.description = req.description;
    if (req.content.has_value())
    {
        tmpl.content = req.content.value();
    }
    else
    {
        tmpl.content = Json::object();
    }

    ApiError error = TemplateDbRepo::create(tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, tmpl);
}

void TemplateApiHandler::clone(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    TemplateCloneRequest req;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &req);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    TemplateDetail tmpl;
    error = TemplateDbRepo::read(id, tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    tmpl.name = req.name;
    tmpl.description = req.description;

    error = TemplateDbRepo::create(tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, tmpl);
}

void TemplateApiHandler::update(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    TemplateUpdateRequest req;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &req);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    TemplateDetail tmpl;
    error = TemplateDbRepo::read(id, tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    tmpl.name = req.name;
    tmpl.description = req.description;
    tmpl.content = req.content;

    error = TemplateDbRepo::update(tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, tmpl);
}

void TemplateApiHandler::remove(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    TemplateDetail tmpl;
    error = TemplateDbRepo::read(id, tmpl);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = TemplateDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, tmpl);
}

