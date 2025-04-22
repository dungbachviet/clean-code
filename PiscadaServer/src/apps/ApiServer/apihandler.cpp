#include "apihandler.h"
#include "apiserverlogger.h"

#include "pimqtt/pimqttclient.h"
#include "pimqtt/pimqttmessage.h"
#include "pimqtt/pimqttpublishproperties.h"

ApiError getUuidFromTopic(const QString &topic, Uuid &id)
{
    QStringList path = topic.split('/');
    if (path.size() < 7)
    {
        return ApiError(ApiError::InvalidRequest, "Topic too short");
    }

    QString idStr = path.at(6);
    id = Uuid(idStr);
    if (!id.isNull())
    {
        return ApiError(ApiError::InvalidRequest, QString("Invalid ID: '%1'").arg(idStr));
    }

    return ApiError();
}

ApiError getStringIdFromTopic(const QString &topic, QString &id)
{
    QStringList path = topic.split('/');
    if (path.size() < 7)
    {
        return ApiError(ApiError::InvalidRequest, "Topic too short");
    }

    id = path.at(6);

    return ApiError();
}

ApiError getControllerIdStringFromTopic(const QString &topic, QString &controllerId)
{
    QStringList path = topic.split('/');
    if (path.size() < 2)
    {
        return ApiError(ApiError::InvalidRequest, "Topic too short");
    }

    controllerId = path.at(1);

    return ApiError();
}

ApiHandler::ApiHandler(PiMqttClient *client) : m_client(client)
{

}

ApiHandler::~ApiHandler() = default;

void ApiHandler::sendResponse(const PiMqttMessage &req, const Json &response)
{
    const PiMqttPublishProperties &properties = req.properties();
    QString responseTopic = properties.responseTopic();
    QString testTopic("test");

    JsonError error;
    QByteArray payload = JsonUtil::serialize(response, &error);

    PiMqttPublishProperties responseProperties;
    responseProperties.setContentType("application/json");
    responseProperties.setPayloadFormatIndicator(PiMqttPayloadFormatIndicator::Utf8);
    responseProperties.setCorrelationData(properties.correlationData());

    PiMqttMessage msg(responseTopic);
    msg.setPayload(payload);
    msg.setProperties(responseProperties);

    qCInfo(ApiServerModuleLogger) << "Sending response to" << responseTopic;
    m_client->publish(msg);

    msg.setTopic(testTopic);

    m_client->publish(msg);
}

void ApiHandler::sendErrorResponse(const PiMqttMessage &msg, const ApiError &error)
{
    Json response = error;

    sendResponse(msg, response);
}

