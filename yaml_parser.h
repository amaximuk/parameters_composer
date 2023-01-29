#ifndef YAML_PARSER_H_
#define YAML_PARSER_H_

#include <string>
#include <vector>
#include "parameters_compiler_types.h"
#include "yaml-cpp/yaml.h"

namespace yaml
{
	class parser
	{
	public:
		static bool parse(const std::string& filename, parameters_compiler::file_info& fi);

	private:
		template<typename T> static bool try_get_yaml_value(const YAML::Node& node, const std::string& name, T& value);

		static bool get_file_info(const YAML::Node& node, parameters_compiler::file_info& fi);
		static bool get_info_info(const YAML::Node& node, parameters_compiler::info_info& ui);
		static bool get_type_info(const YAML::Node& node, const std::vector<parameters_compiler::type_info>& type_infos, parameters_compiler::type_info& ii);
		static bool get_parameter_info(const YAML::Node& node, const std::vector<parameters_compiler::type_info>& type_infos, parameters_compiler::parameter_info& pi);
	};
}



#endif // YAML_PARSER_H_
