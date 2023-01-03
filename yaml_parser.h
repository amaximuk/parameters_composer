#ifndef YAML_PARSER_H_
#define YAML_PARSER_H_

#include <string>
#include <vector>
#include <QDebug>
#include "yaml_parser_types.h"
#include "yaml-cpp/yaml.h"

namespace yaml
{
	class yaml_parser
	{
	private:
		std::map<std::string, std::string> type_type_names_;
		std::map<std::string, std::string> system_type_names_;
		std::map<std::string, std::string> cpp_type_names_;
		std::map<std::string, std::string> xml_type_names_;

	private:
		bool is_batch_;
		file_info file_info_;

	public:
		yaml_parser(bool is_batch);
		~yaml_parser()
		{
			qDebug() << "~";
		}

	public:
		bool parse(const std::string& filename);
		bool get_code(std::list<std::string>& code);
		bool get_wiki(std::list<std::string>& wiki);
		bool get_xml(std::list<std::string>& xml);
		bool get_html(std::list<std::string>& html);
		inline file_info get_file_info() { return file_info_; }

	private:
		template <typename... Args> std::string make_string(Args&&... args);
		template<typename T> bool try_get_yaml_value(const YAML::Node& node, const std::string& name, T& value);

		bool get_file_code(std::list<std::string>& code);
		bool get_file_wiki(std::list<std::string>& wiki);
		bool get_file_xml(std::list<std::string>& xml);
		bool get_file_html(std::list<std::string>& html);

		bool get_type_code(const type_info& ti, std::list<std::string>& code);
		bool get_type_wiki(const type_info& ti, std::list<std::string>& wiki);
		bool get_type_html(const type_info& ti, std::list<std::string>& html);
		bool get_main_code(std::list<std::string>& code);
		bool get_main_wiki(std::list<std::string>& wiki);
		bool get_main_html(std::list<std::string>& html);

		bool get_parameters_xml(const std::vector<parameter_info>& parameters, std::list<std::string>& xml);
		bool get_struct_members_code(const std::vector<parameter_info>& parameters, std::list<std::string>& code);
		bool get_struct_parse_code(const std::vector<parameter_info>& parameters, std::list<std::string>& code);

		bool get_parameter_code(const parameter_info& pi, std::list<std::string>& code);
		bool get_parameter_wiki(const parameter_info& pi, std::list<std::string>& wiki);
		bool get_parameter_xml(const parameter_info& pi, std::list<std::string>& xml);
		bool get_parameter_html(const parameter_info& pi, std::list<std::string>& html);

		bool get_restrictions_code(const parameter_info& pi, bool for_array, std::list<std::string>& code);
		bool get_restrictions_wiki(const parameter_info& pi, std::string& wiki);
		bool get_restrictions_html(const parameter_info& pi, std::string& html);

		bool get_file_info(const YAML::Node& node, file_info& fi);
		bool get_info_info(const YAML::Node& node, info_info& ui);
		bool get_type_info(const YAML::Node& node, const std::vector<type_info>& type_infos, type_info& ti);
		bool get_parameter_info(const YAML::Node& node, const std::vector<type_info>& type_infos, parameter_info& pi);
	};
}
#endif // YAML_PARSER_H_
