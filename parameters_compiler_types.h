#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <regex>

namespace parameters_compiler
{
	const std::vector<std::string> type_type_names_ = {
		"yml",
		"enum"
	};

	const std::vector<std::string> parameter_type_names_ = {
		"unit",
		"path",
		"string",
		"library",
		"double",
		"int",
		"bool",
		"float",
		"int8_t",
		"int16_t",
		"int32_t",
		"int64_t",
		"uint8_t",
		"uint16_t",
		"uint32_t",
		"uint64_t",
		"array<unit>",
		"array<path>",
		"array<string>",
		"array<library>",
		"array<double>",
		"array<int>",
		"array<bool>",
		"array<float>",
		"array<int8_t>",
		"array<int16_t>",
		"array<int32_t>",
		"array<int64_t>",
		"array<uint8_t>",
		"array<uint16_t>",
		"array<uint32_t>",
		"array<uint64_t>"
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
	};

	enum class struct_types
	{
		file_info,
		info_info,
		type_info,
		parameter_info,
		restrictions_info
	};
}