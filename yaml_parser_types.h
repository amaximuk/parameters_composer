#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <regex>

namespace yaml
{
	const std::map<std::string, std::string> type_type_names_ = {
		{"enum", "str"}
	};

	const std::map<std::string, std::string> system_type_names_ = {
		{"unit", "str"},
		{ "path", "str"},
		{ "string", "str"}
	};

	const std::map<std::string, std::string> cpp_type_names_ = {
		{"double", "dbl"},
		{"int", "int"},
		{"bool", "bool"},
		{"std::string", "str"},
		{"float", "dbl"},
		{"int8_t", "int"},
		{"int16_t", "int"},
		{"int32_t", "int"},
		{"int64_t", "int"},
		{"uint8_t", "int"},
		{"uint16_t", "int"},
		{"uint32_t", "int"},
		{"uint64_t", "int"}
	};

	const std::map<std::string, std::string> xml_type_names_ = {
		{"dbl", "double"},
		{"int", "int"},
		{"bool", "bool"},
		{"str", "std::string"}
	};
	
	
	// remove to file yaml_helper!!!!
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


	};


	struct restrictions_info
	{
		// Optional members from yml
		std::string min;
		std::string max;
		std::vector<std::string> set_;
		std::string min_count;
		std::string max_count;
		std::vector<std::string> set_count;
		std::string category;
		std::vector<std::string> ids;
		std::string max_length;
	};

	struct parameter_info
	{
		// Required members from yml
		std::string type;
		std::string name;
		// Optional members from yml
		bool required;
		std::string default_;
		std::string display_name;
		std::string description;
		std::string hint;
		restrictions_info restrictions;
	};

	struct type_info
	{
		// Required members from yml
		std::string name;
		// Optional members from yml
		std::string type;
		std::string description;
		std::vector<std::pair<std::string, std::string>> values;
		std::vector<std::string> includes;
		// Evaluated members
		std::vector<parameter_info> parameters;
	};

	struct info_info
	{
		// Required members from yml
		std::string id;
		// Optional members from yml
		std::string display_name;
		std::string description;
		std::string category;
		std::string hint;
		std::string pictogram;
		std::string author;
		std::string wiki;
	};

	struct file_info
	{
		// Optional members from yml
		std::string format;
		// Evaluated members
		info_info info;
		std::vector<type_info> types;
		std::vector<parameter_info> parameters;

		inline bool get_type(const std::string& name, type_info& type) const
		{
			for (const auto& ti : types)
			{
				if (ti.name == name)
				{
					type = ti;
					return true;
				}
			}
			return false;
		}
	};
}