#include "../server/config.h"
#include "../server/log.h"
#include <yaml-cpp/yaml.h>

ZnetServer::ConfigVar<int>::ptr g_int_value_config = ZnetServer::Config::Lookup("system.port", (int)8080, "system port");
ZnetServer::ConfigVar<std::vector<int> >::ptr g_vec_value_config = ZnetServer::Config::Lookup<std::vector<int> >("system.vec", std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "system vec");

YAML::Node config = YAML::LoadFile("/home/zhounan/NetworkProgramming/ZnetServer/tests/config.yml");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::string key = it->first.as<std::string>();
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << std::string(level * 2, ' ') << "key: " << key;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            print_yaml(*it, level + 1);
        }
    } else if (node.IsScalar()) {
        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << std::string(level * 2, ' ') << "value: " << node.as<std::string>();
    }
}

void test_yaml() {
    print_yaml(config, 0);
}

int main(int argc, char** argv) {
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue() <<  " = 8080";
    auto tmp = ZnetServer::Config::Lookup<int>("system.port");
    tmp->setValue(1000); // 设置值
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue() <<  " = 1000";
    // 测试vector，得到的是vector别直接<<
    std::stringstream ss;
    for (auto& i : g_vec_value_config->getValue()) {
        ss << i << " "; 
    }
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "before vec: " << ss.str();
    ss.str("");

    ZnetServer::Config::LoadFromYaml(config);
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue() <<  " = 8080";
    
    // 测试vector
    for (auto& i : g_vec_value_config->getValue()) {
        ss << i << " "; 
    }
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "after vec: " << ss.str();
    // test_yaml();
    return 0;
}