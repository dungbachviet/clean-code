#ifndef PISCADA_API_SERVER_MODULE_H
#define PISCADA_API_SERVER_MODULE_H

#include "apihandler.h"

#include "pimqtt/pimqttclient.h"

#include "piscadamodule.h"


class ApiServerModule : public PiscadaModule
{
public:
    ModuleStatus construct() override;
    void destruct() override;
    QString getModuleName() override;
    int getCyclePeriod() override;
    enum ModuleKind moduleKind() override;
    void start() override;
    void stop() override;
    void cycle() override;
    void init() override;
    QString getVersion() override;

private slots:
    void onMqttConnected();
    void onMqttDisconnected();
    void onMqttMessageReceived(const PiMqttMessage &msg);

private:
    PiMqttClient *m_client;
    QHash<QString, ApiHandler *> m_handlers;
};

#endif // PISCADA_API_SERVER_MODULE_H
