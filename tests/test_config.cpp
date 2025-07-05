#include "../server/config.h"
#include "../server/log.h"

ZnetServer::ConfigVar<int>::ptr g_int_value_config = ZnetServer::Config::Lookup("system.port", (int)8080, "system port");

int main(int argc, char** argv) {
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue();
    g_int_value_config->setValue(1000);
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue();
    return 0;
}