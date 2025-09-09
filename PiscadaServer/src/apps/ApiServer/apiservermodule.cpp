#include "apiservermodule.h"
#include "apiserverlogger.h"
#include "apiserverinfo.h"
#include "popups/popupapihandler.h"
#include "subsystems/subsystemapihandler.h"
#include "systems/systemapihandler.h"
#include "templates/templateapihandler.h"
#include "views/viewapihandler.h"
#include "documents/documentapihandler.h"
#include "documents/documentservices.h"
#include "systemutils/tempcleaninit.h"

#include "viewtree/viewtreeapihandler.h"

#include "serverinfo.h"

Q_LOGGING_CATEGORY(ApiServerModuleLogger, "piscada.api-server")


ModuleStatus ApiServerModule::construct()
{
    QLoggingCategory::setFilterRules("piscada.api-server.debug=true");

    if (!info.startUp())
    {
        return ModuleStatus::Error("Failed to initialize ApiServerInfo");
    }

    QString mqttHostname = ServerInfo::instance()->getMqttHostnameFromSettings();
    if (mqttHostname.isEmpty())
    {
        mqttHostname = "127.0.0.1";
    }

    int mqttPort = ServerInfo::instance()->getMqttPortFromSettings();
    if (mqttPort < 0)
    {
        mqttPort = 1883;
    }

    m_client = new PiMqttClient(this);
    m_client->setHost(mqttHostname);
    m_client->setPort(mqttPort);
    m_client->setClientId("ApiServer");
    m_client->setCleanSession(true);

    connect(m_client, &PiMqttClient::messageReceived, this, &ApiServerModule::onMqttMessageReceived);
    connect(m_client, &PiMqttClient::connected, this, &ApiServerModule::onMqttConnected);
    connect(m_client, &PiMqttClient::disconnected, this, &ApiServerModule::onMqttDisconnected);

    m_handlers.insert("popups", new PopupApiHandler(m_client));
    m_handlers.insert("subsystems", new SubsystemApiHandler(m_client));
    m_handlers.insert("systems", new SystemApiHandler(m_client));
    m_handlers.insert("templates", new TemplateApiHandler(m_client));
    m_handlers.insert("views", new ViewApiHandler(m_client));
    m_handlers.insert("viewtree", new ViewTreeApiHandler(m_client));
    m_handlers.insert("documents", new DocumentApiHandler(m_client));

    TempCleanInit::startScheduledCleanup(DocumentServices::tempDocumentPath, 60, 24);

    return ModuleStatus::Success();
}

void ApiServerModule::destruct()
{
    info.shutDown();
}

QString ApiServerModule::getModuleName()
{
    return "ApiServer";
}

int ApiServerModule::getCyclePeriod()
{
    return 30'000;
}

PiscadaModule::ModuleKind ApiServerModule::moduleKind()
{
    return APPLICATION;
}

void ApiServerModule::start()
{
    m_client->connectToHost();
}

void ApiServerModule::stop()
{
    m_client->disconnectFromHost();
}

void ApiServerModule::cycle()
{
}

void ApiServerModule::init()
{
}

QString ApiServerModule::getVersion()
{
    return "1.0.0";
}

void ApiServerModule::onMqttConnected()
{
    qCInfo(ApiServerModuleLogger) << "Connected to MQTT broker";

    QString subscriptionTopic = QString("c/%1/m/ApiServer/#").arg(ServerInfo::instance()->getControllerId());
    m_client->subscribe(subscriptionTopic, 0, PiMqttClient::NoLocal);
}

void ApiServerModule::onMqttDisconnected()
{
    qCInfo(ApiServerModuleLogger) << "Disconnected from MQTT broker";
}

void ApiServerModule::onMqttMessageReceived(const PiMqttMessage &msg)
{
    // topic c/<controller_id>/m/ApiServer/<resource>/<action>{/<id>}
    QStringList topic = msg.topic().split('/');
    if (topic.size() < 5)
    {
        return;
    }

    QString resource = topic.at(4);
    auto it = m_handlers.find(resource);
    if (it == m_handlers.end())
    {
        // TODO (Emil): Send error response
        return;
    }

    it.value()->handle(msg);
}

CREATE_PISCADA_MODULE(ApiServerModule)
