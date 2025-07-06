#ifndef __ZNS_CONFIG_H__
#define __ZNS_CONFIG_H__

#include <memory>
#include <string>
#include <map>
#include <list>
#include <vector>
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

// vector<T> -> string
template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

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
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description), m_value(default_value) {}

    T getValue() const { return m_value;}
    void setValue(const T& value) { m_value = value;}

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
            m_value = FromStr()(val);
            return true;
        } catch (const std::exception& e) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "ConfigVar::fromString() error, name=" << m_name
                << " exception: " << e.what();
        }
        return false;
    }
private:
    T m_value;
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
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& value, const std::string& description) {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "Config::Lookup name=" << name << " already exists";
            return tmp;
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
    static void ListAllMember(const YAML::Node& node, const std::string& prefix, std::list<std::pair<std::string, const YAML::Node> >& output);

    private:
        static ConfigVarMap s_datas;
    };
}

#endif