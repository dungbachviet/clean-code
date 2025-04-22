#include "systems/systemapihandler.h"
#include "systems/systemdbrepo.h"
#include "systems/systemtypes.h"
#include "apiserverinfo.h"


SystemApiHandler::SystemApiHandler(PiMqttClient *client)
    : TypedApiHandler<SystemApiHandler>(client)
{
    m_handlers.insert("read", &SystemApiHandler::read);
    m_handlers.insert("list", &SystemApiHandler::list);
    m_handlers.insert("create", &SystemApiHandler::create);
    m_handlers.insert("update", &SystemApiHandler::update);
    m_handlers.insert("delete", &SystemApiHandler::remove);
}

SystemApiHandler::~SystemApiHandler() = default;

void SystemApiHandler::read(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    System system;
    error = SystemDbRepo::read(id, system);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, system);
}

void SystemApiHandler::list(const PiMqttMessage &msg)
{
    QVector<System> systems;
    ApiError error = SystemDbRepo::list(systems);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, systems);
}

void SystemApiHandler::create(const PiMqttMessage &msg)
{
    SystemCreateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    System system;
    system.name = request.name;
    
    ApiError error = SystemDbRepo::create(system);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, system);
}

void SystemApiHandler::update(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    SystemUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    System system;
    system.id = id;
    system.name = request.name;

    error = SystemDbRepo::update(system);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, system);
}

void SystemApiHandler::remove(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    System system;
    error = SystemDbRepo::read(id, system);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    error = SystemDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, system);
}

