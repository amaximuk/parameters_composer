#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <regex>
//#include "base64.h"
//#include "git_commit_hash.h"
#include "yaml_parser.h"

using namespace yaml;

// !!! remove -> last error???
#define ELRF(message) do { std::cout << message << std::endl; return false; } while(0)

bool parser::parse(const std::string& filename, parameters_compiler::file_info& fi)
{
	YAML::Node config{};
	try
	{
		config = YAML::LoadFile(filename);
	}
	catch (...)
	{
		ELRF("File " << filename << " load failed");
	}

	if (!get_file_info(config, fi))
		ELRF("File info parse failed");

	return true;
}

template<typename T>
bool parser::try_get_yaml_value(const YAML::Node& node, const std::string& name, T& value)
{
	if (!node[name])
		return false;
	value = node[name].as<T>();
	return true;
}

bool parser::get_file_info(const YAML::Node& node, parameters_compiler::file_info& fi)
{
	// Optional members from yml
	if (!try_get_yaml_value<std::string>(node, "FILE_FORMAT", fi.format))
		fi.format = "1";

	YAML::Node info = node["INFO"];
	if (!info)
		ELRF("Get INFO node failed");

	if (!get_info_info(info, fi.info))
		ELRF("Unit info parse failed");

	YAML::Node types = node["TYPES"];
	for (const auto& type : types)
	{
		parameters_compiler::type_info ti{};
		if (!get_type_info(type, fi.types, ti))
			ELRF("Type info parse failed");
		fi.types.push_back(std::move(ti));
	}

	YAML::Node parameters = node["PARAMETERS"];
	for (const auto& parameter : parameters)
	{
		parameters_compiler::parameter_info pi{};
		if (!get_parameter_info(parameter, fi.types, pi))
			ELRF("Parameter info parse failed");
		fi.parameters.push_back(std::move(pi));
	}

	return true;
}

bool parser::get_info_info(const YAML::Node& node, parameters_compiler::info_info& ui)
{
	// Required members from yml
	if (!try_get_yaml_value<std::string>(node, "ID", ui.id))
		ELRF("Get unit ID node failed");

	// Optional members from yml
	if (!try_get_yaml_value<std::string>(node, "DISPLAY_NAME", ui.display_name))
		ui.display_name = "";
	if (!try_get_yaml_value<std::string>(node, "DESCRIPTION", ui.description))
		ui.description = "";
	ui.description = std::regex_replace(ui.description, std::regex("\r\n$|\n$"), "");
	if (!try_get_yaml_value<std::string>(node, "CATEGORY", ui.category))
		ui.category = "";
	if (!try_get_yaml_value<std::string>(node, "HINT", ui.hint))
		ui.hint = "";
	if (!try_get_yaml_value<std::string>(node, "PICTOGRAM", ui.pictogram))
		ui.pictogram = "";
	if (!try_get_yaml_value<std::string>(node, "AUTHOR", ui.author))
		ui.author = "";
	if (!try_get_yaml_value<std::string>(node, "WIKI", ui.wiki))
		ui.wiki = "";

	return true;
}

bool parser::get_type_info(const YAML::Node& node, const std::vector<parameters_compiler::type_info>& type_infos, parameters_compiler::type_info& ti)
{
	// Required members from yml
	if (!try_get_yaml_value<std::string>(node, "NAME", ti.name))
		ELRF("Get parameter NAME node failed");

	// Optional members from yml
	if (!try_get_yaml_value<std::string>(node, "TYPE", ti.type))
		ti.type = "yml";
	if (!try_get_yaml_value<std::string>(node, "DESCRIPTION", ti.description))
		ti.description = "";
	ti.description = std::regex_replace(ti.description, std::regex("\r\n$|\n$"), "");

	YAML::Node parameters = node["PARAMETERS"];
	for (const auto& parameter : parameters)
	{
		parameters_compiler::parameter_info pi;
		if (!get_parameter_info(parameter, type_infos, pi))
			ELRF("Get parameter info failed");
		ti.parameters.push_back(std::move(pi));
	}
	YAML::Node values = node["VALUES"];
	for (const auto& value : values)
		ti.values.push_back({ value.first.as<std::string>(), value.second.as<std::string>() });

	YAML::Node includes = node["INCLUDES"];
	for (const auto& include : includes)
		ti.includes.push_back(include.as<std::string>());

	return true;
}

bool parser::get_parameter_info(const YAML::Node& node, const std::vector<parameters_compiler::type_info>& type_infos, parameters_compiler::parameter_info& pi)
{
	// Get required members
	if (!try_get_yaml_value<std::string>(node, "TYPE", pi.type))
		ELRF("Get parameter TYPE node failed");
	if (!try_get_yaml_value<std::string>(node, "NAME", pi.name))
		ELRF("Get parameter NAME node failed");

	// Get optional members
	bool parameter_has_default = true;
	if (!try_get_yaml_value<std::string>(node, "DEFAULT", pi.default_))
	{
		parameter_has_default = false;
		pi.default_ = "";
	}
	if (!try_get_yaml_value<std::string>(node, "DISPLAY_NAME", pi.display_name))
		pi.display_name = "";
	if (!try_get_yaml_value<std::string>(node, "DESCRIPTION", pi.description))
		pi.description = "";
	pi.description = std::regex_replace(pi.description, std::regex("\r\n$|\n$"), "");
	if (!try_get_yaml_value<std::string>(node, "HINT", pi.hint))
		pi.hint = "";
	if (!try_get_yaml_value<bool>(node, "REQUIRED", pi.required))
		pi.required = true;

	YAML::Node restrictions = node["RESTRICTIONS"];
	if (restrictions)
	{
		if (!try_get_yaml_value<std::string>(restrictions, "MIN", pi.restrictions.min))
			pi.restrictions.min = "";
		if (!try_get_yaml_value<std::string>(restrictions, "MAX", pi.restrictions.max))
			pi.restrictions.max = "";
		YAML::Node required_set = restrictions["SET"];
		for (const auto& v : required_set)
			pi.restrictions.set_.push_back(v.as<std::string>());
		if (!try_get_yaml_value<std::string>(restrictions, "MIN_COUNT", pi.restrictions.min_count))
			pi.restrictions.min_count = "";
		if (!try_get_yaml_value<std::string>(restrictions, "MAX_COUNT", pi.restrictions.max_count))
			pi.restrictions.max_count = "";
		YAML::Node required_set_count = restrictions["SET_COUNT"];
		for (const auto& v : required_set_count)
			pi.restrictions.set_count.push_back(v.as<std::string>());
		if (!try_get_yaml_value<std::string>(restrictions, "CATEGORY", pi.restrictions.category))
			pi.restrictions.category = "";
		YAML::Node required_ids = restrictions["IDS"];
		for (const auto& v : required_ids)
			pi.restrictions.ids.push_back(v.as<std::string>());
		if (!try_get_yaml_value<std::string>(restrictions, "MAX_LENGTH", pi.restrictions.max_length))
			pi.restrictions.max_length = "";
	}

	return true;
}
