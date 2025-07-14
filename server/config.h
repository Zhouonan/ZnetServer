#ifndef __ZNS_CONFIG_H__
#define __ZNS_CONFIG_H__

#include <memory>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include "log.h"

namespace ZnetServer {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string &name, const std::string& desciption = "")
        :m_name(name)
        ,m_description(desciption) {}
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }
    
    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
protected:
    std::string m_name;
    std::string m_description;
};

template<class F, class T>
class LexicalCast {
public:
    T operator()(const F& v) {
        return boost::lexical_cast<T>(v);
    }
};

//vector<T>, list<T>, set<T>, unordered_set<T> -> string
#define CAST_TO_STRING(mytype) \
    template<class T> \
    class LexicalCast<mytype<T>, std::string> { \
    public: \
        std::string operator()(const mytype<T>& v) { \
            YAML::Node node; \
            for (auto& i : v) { \
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i))); \
            } \
            std::stringstream ss; \
            ss << node; \
            return ss.str(); \
        } \
    }; \

CAST_TO_STRING(std::vector)
CAST_TO_STRING(std::list)
CAST_TO_STRING(std::set)
CAST_TO_STRING(std::unordered_set)

#undef CAST_TO_STRING

// string -> vector<T>
template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        for (size_t i = 0; i < node.size(); ++i) {
            // vec.push_back(LexicalCast<std::string, T>()(i.Scalar()));
            std::stringstream ss;
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

// string -> list<T>
template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::list<T> l;
        for(size_t i = 0; i < node.size(); i ++) {
            std::stringstream ss;
            ss << node[i];
            l.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return l;
    }
};

// string -> set<T>
template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& s) {
        YAML::Node node = YAML::Load(s);
        std::set<T> se;
        for(size_t i = 0; i < node.size(); i ++) {
            std::stringstream ss;
            ss << node[i];
            se.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return se;
    }
};

// string -> unordered_set<T>
template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& s) {
        YAML::Node node = YAML::Load(s);
        std::unordered_set<T> se;
        for(size_t i = 0; i < node.size(); i ++) {
            std::stringstream ss;
            ss << node[i];
            se.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return se;
    }
};

// string -> map<string, T>
template<class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    std::map<std::string, T> operator()(const std::string& s) {
        YAML::Node node = YAML::Load(s);
        std::map<std::string, T> m;
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::stringstream ss;
            ss << it->second;
            m[it->first.Scalar()] = LexicalCast<std::string, T>()(ss.str());
        }
        return m;
    }
};

// map<string, T> -> string
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& m) {
        YAML::Node node;
        for (auto& i : m) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// string -> unordered_map<string, T>
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& s) {
        YAML::Node node = YAML::Load(s);
        std::unordered_map<std::string, T> m;
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::stringstream ss;
            ss << it->second;
            m[it->first.Scalar()] = LexicalCast<std::string, T>()(ss.str());
        }
        return m;
    }
};

//unordered_map<string, T> -> string
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& m) {
        YAML::Node node;
        for (auto& i : m) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 配置项类
 * 
 * @tparam T 
 * @tparam FromStr 
 * @tparam ToStr 
 */
template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T& newValue, const T& oldValue)> OnChangeCallback;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description), m_value(default_value) {}

    T getValue() const { return m_value;}
    void setValue(const T& value) { 
        if (m_value == value) {
            return;
        }
        T old_value = m_value;
        m_value = value;
        for (auto& cb : m_cbs) {
            cb.second(value, old_value);
        }
    }

    std::string toString() override { 
        try {
            return ToStr()(m_value);
        } catch (const std::exception& e) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "ConfigVar::toString() error, name=" << m_name
                << " exception: " << e.what();
        }
        return "";
    }
    bool fromString(const std::string& val) override { 
        try {
            setValue(FromStr()(val));
            return true;
        } catch (const std::exception& e) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "ConfigVar::fromString() error, name=" << m_name
                << " exception: " << e.what();
        }
        return false;
    }
    void addListener(int fd, OnChangeCallback cb) {
        m_cbs[fd] = cb;
    }
    void removeListener(int fd) {
        m_cbs.erase(fd);
    }
    OnChangeCallback getListener(int fd) {
        auto it = m_cbs.find(fd);
        return it == m_cbs.end() ? nullptr : it->second;
    }
private:
    T m_value;
    std::map<int, OnChangeCallback> m_cbs;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    /**
     * @brief 返回基类指针，适用于不知道具体类型时的通用查找。
     * 
     * @param name 
     * @return ConfigVarBase::ptr 
     */
    static ConfigVarBase::ptr LookupBase(const std::string& name) {
        auto it = s_datas.find(name);
        return it == s_datas.end() ? nullptr : it->second;
    }
    // 已知类型查找，省去了手动 dynamic_pointer_cast 的麻烦
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto tmp = LookupBase(name);
        if (tmp) {
            return std::dynamic_pointer_cast<ConfigVar<T>>(tmp);
        }
        return nullptr;
    }
    /**
     * @brief 查找配置项，如果存在则返回已存在的配置项，否则创建新的配置项
     * 
     * @tparam T 
     * @param name 
     * @param value 
     * @param description 
     * @return typename ConfigVar<T>::ptr 
     */
    template<class T>
    static typename ConfigVar<T>::ptr Create(const std::string& name, const T& value, const std::string& description) {
        // 不能直接通过LookUp查找后通过判断tmp是否为nullptr来判断是否存在，可能会出现key相同而类型不同导致混乱的情况
        auto tmp = LookupBase(name);
        if (tmp) {
            auto casted = std::dynamic_pointer_cast<ConfigVar<T>>(tmp);
            if (casted) {
                ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "Config::Create name=" << name << " already exists";
                return casted;
            } else { // 类型不匹配，抛出异常
                throw std::logic_error("Config::Create: Name Conflict! " + name + " already exist, but type is not " + typeid(T).name());
            }
        }
        // 检查name是否合法
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._") != std::string::npos) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Config::Lookup name is invalid, name=" << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr var(new ConfigVar<T>(name, value, description));
        s_datas[name] = var;
        return var;
    }
    static void LoadFromYaml(const YAML::Node& node);
    static void LoadFromYaml(const std::string& filename) {
        YAML::Node root = YAML::LoadFile(filename);
        LoadFromYaml(root);
    }
    static void ListAllYamlMember(const YAML::Node& node, const std::string& prefix, std::list<std::pair<std::string, const YAML::Node> >& output);

    private:
        static ConfigVarMap s_datas;
    };
}

#endif