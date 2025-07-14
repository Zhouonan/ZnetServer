#include "../server/config.h"
#include "../server/log.h"
#include <yaml-cpp/yaml.h>

ZnetServer::ConfigVar<int>::ptr g_int_value_config = ZnetServer::Config::Create("system.port", (int)8080, "system port");
ZnetServer::ConfigVar<std::vector<int> >::ptr g_vec_value_config = ZnetServer::Config::Create<std::vector<int> >("system.vec", std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "system vec");
ZnetServer::ConfigVar<std::list<int> >::ptr g_list_value_config = ZnetServer::Config::Create<std::list<int> >("system.list", std::list<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "system list");
ZnetServer::ConfigVar<std::set<int> >::ptr g_set_value_config = ZnetServer::Config::Create<std::set<int> >("system.set", std::set<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "system set");
ZnetServer::ConfigVar<std::unordered_set<int> >::ptr g_uset_value_config = ZnetServer::Config::Create<std::unordered_set<int> >("system.unordered_set", std::unordered_set<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "system unordered_set");
ZnetServer::ConfigVar<std::map<std::string, int> >::ptr g_map_value_config = ZnetServer::Config::Create<std::map<std::string, int> >("system.map", std::map<std::string, int>{{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}, {"f", 6}, {"g", 7}, {"h", 8}, {"i", 9}}, "system map");
ZnetServer::ConfigVar<std::unordered_map<std::string, int> >::ptr g_umap_value_config = ZnetServer::Config::Create<std::unordered_map<std::string, int> >("system.unordered_map", std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}, {"f", 6}, {"g", 7}, {"h", 8}, {"i", 9}}, "system unordered_map");
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
    // 设置需要遍历的stl的测试宏
    #define STL_TEST_1(g_var, name, prefix) \
        { \
            std::stringstream ss; \
            for (auto& i : g_var->getValue()) { \
                ss << i << " "; \
            } \
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << #prefix " " #name ": " << ss.str(); \
        } 
    
    #define STL_TEST_2(g_var, name, prefix) \
        { \
            std::stringstream ss; \
            for (auto& i : g_var->getValue()) { \
                ss << i.first << ": " << i.second << " "; \
            } \
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << #prefix " " #name ": " << ss.str(); \
        }

    // vector test
    STL_TEST_1(g_vec_value_config, vec, before);
    // list test
    STL_TEST_1(g_list_value_config, list, before);
    // set test
    STL_TEST_1(g_set_value_config, set, before);
    // unordered_set test
    STL_TEST_1(g_uset_value_config, unordered_set, before);
    // map test
    STL_TEST_2(g_map_value_config, map, before);
    // unordered_map test
    STL_TEST_2(g_umap_value_config, unordered_map, before);

    // AFTER LOAD CONFIG
    ZnetServer::Config::LoadFromYaml(config);
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue() <<  " = 8080";
    STL_TEST_1(g_vec_value_config, vec, after);
    STL_TEST_1(g_list_value_config, list, after);
    STL_TEST_1(g_set_value_config, set, after);
    STL_TEST_1(g_uset_value_config, unordered_set, after);
    STL_TEST_2(g_map_value_config, map, after);
    STL_TEST_2(g_umap_value_config, unordered_map, after);

    // test create: same name but different type. throw exception
    auto tmp2 = ZnetServer::Config::Create<std::string>("system.port", "1000", "system port");
    // test_yaml();
    return 0;
}