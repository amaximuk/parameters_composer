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

		//static std::pair<std::vector<yaml::type_info>::iterator, bool> get_type(yaml::file_info& fi, const std::string& name, type_info& type)
		static auto get_type_info(yaml::file_info& fi, const std::string& type)
		{
			const auto it = std::find_if(fi.types.begin(), fi.types.end(), [type](const auto& t) { return t.name == name; });
			return std::make_pair(it, it != fi.types.end());
		}

		static auto get_parameter_info(yaml::file_info& fi, const std::string& type)
		{
			const auto it = std::find_if(fi.types.begin(), fi.types.end(), [type](const auto& t) { return t.name == name; });
			return std::make_pair(it, it != fi.types.end());
		}

        static bool save_parameters(yaml::file_info& fi, const std::string& type, const yaml::parameter_info& pi)
        {
            if (type == "Main")
            {
                for (auto& p : fi.parameters)
                {
                    if (p.name == pi.name)
                    {
                        p = pi;
                        break;
                    }
                }
            }
            else
            {
                for (auto& t : fileInfo_.types)
                {
                    if (QString::fromStdString(t.name) == type)
                    {
                        for (auto& p : t.parameters)
                        {
                            if (p.name == pi.name)
                            {
                                p = pi;
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            return true;
        }

	};

	//auto cc = yaml::helper::get_type(fi, "CHANNEL");
	//std::cout << cc.second;

}

#endif // YAML_HELPER_H_
