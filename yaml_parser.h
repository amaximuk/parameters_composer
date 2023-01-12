#ifndef YAML_PARSER_H_
#define YAML_PARSER_H_

#include <string>
#include <vector>
#include <QDebug>
#include "yaml_parser_types.h"
#include "yaml-cpp/yaml.h"

namespace yaml
{
	class parser
	{
	public:
		static bool parse(const std::string& filename, file_info& fi);

	private:
		template <typename... Args> static std::string make_string(Args&&... args);
		template<typename T> static bool try_get_yaml_value(const YAML::Node& node, const std::string& name, T& value);

		static bool get_file_info(const YAML::Node& node, file_info& fi);
		static bool get_info_info(const YAML::Node& node, info_info& ui);
		static bool get_type_info(const YAML::Node& node, const std::vector<type_info>& type_infos, type_info& ti);
		static bool get_parameter_info(const YAML::Node& node, const std::vector<type_info>& type_infos, parameter_info& pi);
	};
}
#endif // YAML_PARSER_H_
