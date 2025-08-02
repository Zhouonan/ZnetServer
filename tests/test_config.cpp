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

struct Person {
    std::string m_name;
    int m_age;
    int m_gender;

    std::string toString() const {
        std::stringstream ss;
        ss << "name: " << m_name << ", age: " << m_age << ", gender: " << (m_gender == 1 ? "male" : "female");
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
        os << person.toString();
        return os;
    }

    bool operator==(const Person& other) const {
        return m_name == other.m_name && m_age == other.m_age && m_gender == other.m_gender;
    }

    bool operator!=(const Person& other) const {
        return !(*this == other);
    }
};

namespace ZnetServer {
    template<>
    class LexicalCast<std::string, Person> {
        public:
            Person operator()(const std::string& v) {
                YAML::Node node = YAML::Load(v);
                Person person;
                person.m_name = node["name"].as<std::string>();
                person.m_age = node["age"].as<int>();
                person.m_gender = node["gender"].as<int>();
                return person;
            }
    };

    template<>
    class LexicalCast<Person, std::string> {
        public:
            std::string operator()(const Person& p) {
                YAML::Node node;
                node["name"] = p.m_name;
                node["age"] = p.m_age;
                node["gender"] = p.m_gender;
                std::stringstream ss;
                ss << node;
                return ss.str();
            }
    };
}

ZnetServer::ConfigVar<Person>::ptr g_person_config = ZnetServer::Config::Create<Person>("struct.person", Person{"John", 20, 1}, "system person");
ZnetServer::ConfigVar<std::map<std::string, Person>>::ptr g_str_person_config = ZnetServer::Config::Create<std::map<std::string, Person>>("struct.str_person", std::map<std::string, Person>{{"A", Person{"Mike", 20, 1}}}, "str person");

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
    YAML::Node config = YAML::LoadFile("/home/zhounan/NetworkProgramming/ZnetServer/tests/config.yml");
    print_yaml(config, 0);
}

void test_log() {
    auto logger_mgr = ZnetServer::LoggerMgr::GetInstance();
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << logger_mgr->toYamlString();
    ZnetServer::Logger::ptr lt = logger_mgr->createLogger("test");
    lt->addAppender(ZnetServer::LogAppender::ptr(new ZnetServer::FileLogAppender("../log.yml")));
    std::cout << " ================ create logger test =================" << std::endl;
    ZNS_LOG_INFO(lt) << logger_mgr->toYamlString();
    std::cout << " ================ load config =================" << std::endl;
    ZnetServer::Config::LoadFromYaml("/home/zhounan/NetworkProgramming/ZnetServer/tests/config.yml");
    ZNS_LOG_INFO(lt) << logger_mgr->toYamlString();
}

void test_type() {
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
    // self-defined config
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "person_config: " << g_person_config->getValue();
    // map<string, Person> test
    STL_TEST_2(g_str_person_config, str_person_map, before);

    // test add listener
    g_int_value_config->addListener(1, [](const int& newValue, const int& oldValue) {
        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config has been changed: " << oldValue << " -> " << newValue;
    });
    g_int_value_config->setValue(1000);
    g_int_value_config->setValue(10000);
    g_person_config->addListener(2, [](const Person& newValue, const Person& oldValue) {
        ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "person_config has been changed: " << oldValue << " -> " << newValue;
    });
    
    // AFTER LOAD CONFIG
    ZnetServer::Config::LoadFromYaml("/home/zhounan/NetworkProgramming/ZnetServer/tests/config.yml");
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "int_value_config: " << g_int_value_config->getValue() <<  " = 8080";
    STL_TEST_1(g_vec_value_config, vec, after);
    STL_TEST_1(g_list_value_config, list, after);
    STL_TEST_1(g_set_value_config, set, after);
    STL_TEST_1(g_uset_value_config, unordered_set, after);
    STL_TEST_2(g_map_value_config, map, after);
    STL_TEST_2(g_umap_value_config, unordered_map, after);
    ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "person_config: " << g_person_config->getValue();
    STL_TEST_2(g_str_person_config, str_person_map, after);
}

int main(int argc, char** argv) {
    
    // test create: same name but different type. throw exception
    // auto tmp2 = ZnetServer::Config::Create<std::string>("system.port", "1000", "system port");

    // test_yaml();
    // test_type();
    test_log();
    return 0;
}