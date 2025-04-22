#include "subsystems/subsystemapihandler.h"
#include "subsystems/subsystemtypes.h"
#include "subsystems/subsystemdbrepo.h"


SubsystemApiHandler::SubsystemApiHandler(PiMqttClient *client)
    : TypedApiHandler<SubsystemApiHandler>(client)
{
    m_handlers.insert("read", &SubsystemApiHandler::read);
    m_handlers.insert("list", &SubsystemApiHandler::list);
    m_handlers.insert("create", &SubsystemApiHandler::create);
    m_handlers.insert("update", &SubsystemApiHandler::update);
    m_handlers.insert("delete", &SubsystemApiHandler::remove);
}

SubsystemApiHandler::~SubsystemApiHandler() = default;

void SubsystemApiHandler::read(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    Subsystem subsystem;
    error = SubsystemDbRepo::read(id, subsystem);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, subsystem);
}

void SubsystemApiHandler::list(const PiMqttMessage &msg)
{
    QVector<Subsystem> subsystems;
    ApiError error = SubsystemDbRepo::list(subsystems);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, subsystems);
}

void SubsystemApiHandler::create(const PiMqttMessage &msg)
{
    SubsystemCreateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    Subsystem subsystem;
    subsystem.name = request.name;
    subsystem.parentId = request.parentId;

    ApiError error = SubsystemDbRepo::create(subsystem);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, subsystem);
}

void SubsystemApiHandler::update(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    SubsystemUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    Subsystem subsystem;
    error = SubsystemDbRepo::read(id, subsystem);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    subsystem.name = request.name;
    subsystem.parentId = request.parentId;

    error = SubsystemDbRepo::update(subsystem);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, subsystem);
}

void SubsystemApiHandler::remove(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    Subsystem subsystem;
    error = SubsystemDbRepo::read(id, subsystem);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = SubsystemDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, subsystem);
}

