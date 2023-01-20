#ifndef YAML_HELPER_H_
#define YAML_HELPER_H_

#include <string>
#include <vector>
#include <QDebug>
#include "yaml_parser_types.h"
#include "yaml-cpp/yaml.h"

namespace yaml
{
	class helper
	{
	public:
		static bool is_inner_type(std::string name)
		{
			if (system_type_names_.find(name) != system_type_names_.end())
				return true;
			if (cpp_type_names_.find(name) != cpp_type_names_.end())
				return true;

			return false;
		}

		static yaml::type_info* get_type_info(yaml::file_info& fi, const std::string& type)
		{
			const auto it = std::find_if(fi.types.begin(), fi.types.end(), [type](const auto& ti) { return ti.name == type; });
            if (it == fi.types.end())
                return nullptr;
            else
			    return &(*it);
		}

        static bool set_type_info(yaml::file_info& fi, const std::string& type, const yaml::type_info& ti, bool exclude_parameters)
        {
            yaml::type_info* pti = get_type_info(fi, type);
            if (!pti) return false;

            if (exclude_parameters)
            {
                yaml::type_info tit(ti);
                tit.parameters = pti->parameters;
                *pti = tit;
            }
            else
            {
                *pti = ti;
            }
            return true;
        }

		static std::vector<yaml::parameter_info>* get_parameters(yaml::file_info& fi, const std::string& type)
		{
            std::vector<yaml::parameter_info>* pvect = nullptr;
            if (type == "Main")
            {
                pvect = &fi.parameters;
            }
            else
            {
                auto pti = get_type_info(fi, type);
                if (pti) pvect = &pti->parameters;
            }
            return pvect;
		}

		static yaml::parameter_info* get_parameter_info(yaml::file_info& fi, const std::string& type, const std::string& name)
		{
            yaml::parameter_info* ppi = nullptr;
			auto pvect = get_parameters(fi, type);
            if (pvect)
            {
                auto it = std::find_if(pvect->begin(), pvect->end(), [name](const auto& pi) { return pi.name == name; });
                if (it != pvect->end())
                    ppi = &(*it);
            }
			return ppi;
		}

        static bool set_parameter(yaml::file_info& fi, const std::string& type, const yaml::parameter_info& pi)
        {
            auto ppi = get_parameter_info(fi, type, pi.name);
            if (ppi)
                *ppi = pi;

            return true;
        }

        static std::vector<std::string> get_type_names(yaml::file_info& fi)
        {
            std::vector<std::string> result;
            for (const auto& v : fi.types)
                result.push_back(v.name);
            return result;
        }

	};
}

#endif // YAML_HELPER_H_
