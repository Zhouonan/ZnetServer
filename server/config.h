#ifndef __ZNS_CONFIG_H__
#define __ZNS_CONFIG_H__

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

    virtual std::string to
}

}

#endif