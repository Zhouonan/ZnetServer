#include "config.h"

namespace ZnetServer {

void Config::ListAllYamlMember(const YAML::Node& node, const std::string& prefix, std::list<std::pair<std::string, const YAML::Node> >& output) {
    // 为什么用list，不用map？
    // prefix是否合法
    if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._") != std::string::npos) {
        ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Config::ListAllMember prefix is invalid, prefix=" << prefix;
        return;
    }
    // std::cout << "Config::ListAllMember " << prefix << " = " << node << std::endl;
    // 每一个节点都要push_back？
    output.push_back(std::make_pair(prefix, node));

    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            ListAllYamlMember(it->second, prefix.empty() ? it->first.Scalar() : (prefix + "." + it->first.Scalar()), output);
        }
    } 
}

void Config::LoadFromYaml(const YAML::Node& node) {
    std::list<std::pair<std::string, const YAML::Node> > all_members;
    ListAllYamlMember(node, "", all_members);

    for (auto& i : all_members) {
        auto tmp = LookupBase(i.first);
        if (tmp) {
            if (i.second.IsScalar()) {
                tmp->fromString(i.second.Scalar());
                // std::cout << "Config::LoadFromYaml " << i.first << " = " << i.second.Scalar() << std::endl;
            } else { // 序列要怎么办？
                std::stringstream ss;
                ss << i.second;
                tmp->fromString(ss.str());
                // std::cout << "Config::LoadFromYaml " << i.first << " = " << ss.str() << std::endl;
            }
        }
    }
}

}   