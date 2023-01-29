#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <regex>
//#include "base64.h"
//#include "git_commit_hash.h"
#include "yaml_writer.h"

using namespace yaml;

// !!! remove -> last error???
#define ELRF(message) do { std::cout << message << std::endl; return false; } while(0)

bool writer::write(const std::string& filename, parameters_compiler::file_info& fi)
{
	YAML::Emitter emitter;

	if (!write_file_info(emitter, fi))
		ELRF("File info emit failed");

	std::ofstream of(filename);
	if (!of.is_open())
		ELRF("File " << filename << " open failed");

	of.write(emitter.c_str(), emitter.size());
	of.close();

	return true;
}

bool writer::write_file_info(YAML::Emitter& emitter, const parameters_compiler::file_info& fi)
{
    emitter << YAML::BeginMap;
    write_info_info(emitter, fi.info);

    if (fi.types.size() > 0)
    {
        emitter << YAML::Key << "TYPES";
        emitter << YAML::Value << YAML::BeginSeq;
        for (const auto& ti : fi.types)
            write_type_info(emitter, ti);
        emitter << YAML::EndSeq;
    }

    if (fi.parameters.size() > 0)
    {
        emitter << YAML::Key << "PARAMETERS";
        emitter << YAML::Value << YAML::BeginSeq;
        for (const auto& pi : fi.parameters)
            write_parameter_info(emitter, pi);
        emitter << YAML::EndSeq;
    }

    emitter << YAML::EndMap;

    return true;
}

bool writer::write_info_info(YAML::Emitter& emitter, const parameters_compiler::info_info& ii)
{
    emitter << YAML::Key << "INFO";
    emitter << YAML::Value << YAML::BeginMap;

    emitter << YAML::Key << "ID";
    emitter << YAML::Value << ii.id;
    if (ii.display_name != "")
    {
        emitter << YAML::Key << "DISPLAY_NAME";
        emitter << YAML::Value << ii.display_name;
    }
    if (ii.category != "")
    {
        emitter << YAML::Key << "CATEGORY";
        emitter << YAML::Value << ii.category;
    }
    if (ii.description != "")
    {
        emitter << YAML::Key << "DESCRIPTION";
        emitter << YAML::Value << YAML::Literal << ii.description;
    }
    if (ii.pictogram != "")
    {
        emitter << YAML::Key << "PICTOGRAM";
        emitter << YAML::Value << ii.pictogram;
    }
    if (ii.hint != "")
    {
        emitter << YAML::Key << "HINT";
        emitter << YAML::Value << ii.hint;
    }
    if (ii.author != "")
    {
        emitter << YAML::Key << "AUTHOR";
        emitter << YAML::Value << ii.author;
    }
    if (ii.wiki != "")
    {
        emitter << YAML::Key << "WIKI";
        emitter << YAML::Value << ii.wiki;
    }

    emitter << YAML::EndMap;

    return true;
}

bool writer::write_type_info(YAML::Emitter& emitter, const parameters_compiler::type_info& ti)
{
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "NAME";
    emitter << YAML::Value << ti.name;
    if (ti.type != "" && ti.type != "yml")
    {
        emitter << YAML::Key << "TYPE";
        emitter << YAML::Value << ti.type;
    }
    if (ti.description != "")
    {
        emitter << YAML::Key << "DESCRIPTION";
        emitter << YAML::Value << YAML::Literal << ti.description;
    }
    if (ti.values.size() > 0)
    {
        emitter << YAML::Key << "VALUES";
        emitter << YAML::Value << YAML::BeginMap;
        for (const auto& v : ti.values)
        {
            emitter << YAML::Key << v.first;
            emitter << YAML::Value << v.second;
        }
        emitter << YAML::EndMap;
    }
    if (ti.includes.size() > 0)
    {
        emitter << YAML::Key << "INCLUDES";
        emitter << YAML::Value << ti.includes;
    }
    if (ti.parameters.size() > 0)
    {
        emitter << YAML::Key << "PARAMETERS";
        emitter << YAML::Value << YAML::BeginSeq;
        for (const auto& pi : ti.parameters)
            write_parameter_info(emitter, pi);
        emitter << YAML::EndSeq;
    }
    emitter << YAML::EndMap;

    return true;
}

bool writer::write_parameter_info(YAML::Emitter& emitter, const parameters_compiler::parameter_info& pi)
{
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "NAME";
    emitter << YAML::Value << pi.name;
    emitter << YAML::Key << "TYPE";
    emitter << YAML::Value << pi.type;
    if (pi.display_name != "")
    {
        emitter << YAML::Key << "DISPLAY_NAME";
        emitter << YAML::Value << pi.display_name;
    }
    if (pi.description != "")
    {
        emitter << YAML::Key << "DESCRIPTION";
        emitter << YAML::Value << YAML::Literal << pi.description;
    }
    if (pi.required != true)
    {
        emitter << YAML::Key << "REQUIRED";
        emitter << YAML::Value << YAML::TrueFalseBool << pi.required;
    }
    if (pi.default_ != "")
    {
        emitter << YAML::Key << "DEFAULT";
        emitter << YAML::Value << pi.default_;
    }
    if (pi.hint != "")
    {
        emitter << YAML::Key << "HINT";
        emitter << YAML::Value << pi.hint;
    }

    if (pi.restrictions.min != "" || pi.restrictions.max != "" || pi.restrictions.set_.size() > 0 ||
        pi.restrictions.min_count != "" || pi.restrictions.max_count != "" || pi.restrictions.set_count.size() > 0 ||
        pi.restrictions.category != "" || pi.restrictions.ids.size() > 0 || pi.restrictions.max_length != "")
    {
        emitter << YAML::Key << "RESTRICTIONS";
        emitter << YAML::Value << YAML::BeginMap;
        if (pi.restrictions.min != "")
        {
            emitter << YAML::Key << "MIN";
            emitter << YAML::Value << pi.restrictions.min;
        }
        if (pi.restrictions.max != "")
        {
            emitter << YAML::Key << "MAX";
            emitter << YAML::Value << pi.restrictions.max;
        }
        if (pi.restrictions.set_.size() > 0)
        {
            emitter << YAML::Key << "SET";
            emitter << YAML::Value << YAML::Flow << pi.restrictions.set_;
        }
        if (pi.restrictions.min_count != "")
        {
            emitter << YAML::Key << "MIN_COUNT";
            emitter << YAML::Value << pi.restrictions.min_count;
        }
        if (pi.restrictions.max_count != "")
        {
            emitter << YAML::Key << "MAX_COUNT";
            emitter << YAML::Value << pi.restrictions.max_count;
        }
        if (pi.restrictions.set_count.size() > 0)
        {
            emitter << YAML::Key << "SET_COUNT";
            emitter << YAML::Value << YAML::Flow << pi.restrictions.set_count;
        }
        if (pi.restrictions.category != "")
        {
            emitter << YAML::Key << "CATEGORY";
            emitter << YAML::Value << pi.restrictions.category;
        }
        if (pi.restrictions.ids.size() > 0)
        {
            emitter << YAML::Key << "IDS";
            emitter << YAML::Value << YAML::Flow << pi.restrictions.ids;
        }
        if (pi.restrictions.max_length != "")
        {
            emitter << YAML::Key << "MAX_LENGTH";
            emitter << YAML::Value << pi.restrictions.max_length;
        }
        emitter << YAML::EndMap;
    }

    emitter << YAML::EndMap;

    return true;
}
