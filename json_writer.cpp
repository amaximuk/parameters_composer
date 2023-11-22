#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <regex>
//#include "base64.h"
//#include "git_commit_hash.h"
#include "encoding_cp1251.h"
#include "json_writer.h"

using namespace json;

inline std::string cp1251_to_utf8(std::string x)
{
    return x;
}
// !!! remove -> last error???
#define ELRF(message) do { std::cout << message << std::endl; return false; } while(0)

bool writer::write(const std::string& filename, parameters_compiler::file_info& fi)
{
    Json::Value mainObject;
	if (!write_file_info(mainObject, fi))
		ELRF("File info emit failed");

	std::ofstream of(filename);
	if (!of.is_open())
		ELRF("File " << filename << " open failed");

    Json::StyledWriter styledWriter;
    of << styledWriter.write(mainObject);
	of.close();

 //   // lastly we created a JSON document and set mainObject as object of document
 //   QJsonDocument jsonDoc;
 //   jsonDoc.setObject(mainObject);

 //   // Write our jsondocument as json with JSON format
 //   QByteArray ba = jsonDoc.toJson();
	//of.write(ba.data(), ba.size());
	//of.close();

	return true;
}

bool writer::write_file_info(Json::Value& emitter, const parameters_compiler::file_info& fi)
{
    write_info_info(emitter, fi.info);
    if (fi.types.size() > 0)
    {
        Json::Value types(Json::arrayValue);
        for (const auto& ti : fi.types)
            write_type_info(types, ti);
        emitter["TYPES"] = types;
    }
    if (fi.parameters.size() > 0)
    {
        Json::Value parameters(Json::arrayValue);
        for (const auto& pi : fi.parameters)
            write_parameter_info(parameters, pi);
        emitter["PARAMETERS"] = parameters;
    }

    return true;
}

bool writer::write_info_info(Json::Value& emitter, const parameters_compiler::info_info& ii)
{
    Json::Value info;
    info["ID"] = cp1251_to_utf8(ii.id);
    if (ii.display_name != "")
        info["DISPLAY_NAME"] = cp1251_to_utf8(ii.display_name);
    if (ii.category != "")
        info["CATEGORY"] = cp1251_to_utf8(ii.category);
    if (ii.description != "")
        info["DESCRIPTION"] = cp1251_to_utf8(ii.description);
    if (ii.pictogram != "")
        info["PICTOGRAM"] = cp1251_to_utf8(ii.pictogram);
    if (ii.hint != "")
        info["HINT"] = cp1251_to_utf8(ii.hint);
    if (ii.author != "")
        info["AUTHOR"] = cp1251_to_utf8(ii.author);
    if (ii.wiki != "")
        info["WIKI"] = cp1251_to_utf8(ii.wiki);
    emitter["INFO"] = info;

    return true;
}

bool writer::write_type_info(Json::Value& emitter, const parameters_compiler::type_info& ti)
{
    Json::Value info;
    info["NAME"] = cp1251_to_utf8(ti.name);
    if (ti.type != "" && ti.type != "yml")
        info["TYPE"] = cp1251_to_utf8(ti.type);
    if (ti.description != "")
        info["DESCRIPTION"] = cp1251_to_utf8(ti.description);
    if (ti.values.size() > 0)
    {
        Json::Value values;
        for (const auto& v : ti.values)
            values[cp1251_to_utf8(v.first)] = cp1251_to_utf8(v.second);
        info["VALUES"] = values;
    }
    if (ti.includes.size() > 0)
    {
        Json::Value includes(Json::arrayValue);
        for (const auto& v : ti.includes)
            includes.append(cp1251_to_utf8(v));
        info["INCLUDES"] = includes;
    }
    if (ti.parameters.size() > 0)
    {
        Json::Value parameters(Json::arrayValue);
        for (const auto& pi : ti.parameters)
            write_parameter_info(parameters, pi);
        info["PARAMETERS"] = parameters;
    }
    emitter.append(info);
    
    return true;
}

bool writer::write_parameter_info(Json::Value& emitter, const parameters_compiler::parameter_info& pi)
{
    Json::Value info;
    info["NAME"] = cp1251_to_utf8(pi.name);
    info["TYPE"] = cp1251_to_utf8(pi.type);
    if (pi.display_name != "")
        info["DISPLAY_NAME"] = cp1251_to_utf8(pi.display_name);
    if (pi.description != "")
        info["DESCRIPTION"] = cp1251_to_utf8(pi.description);
    if (pi.required != true)
        info["REQUIRED"] = pi.required;
    if (pi.default_ != "")
        info["DEFAULT"] = cp1251_to_utf8(pi.default_);
    if (pi.hint != "")
        info["HINT"] = cp1251_to_utf8(pi.hint);

    if (pi.restrictions.min != "" || pi.restrictions.max != "" || pi.restrictions.set_.size() > 0 ||
        pi.restrictions.min_count != "" || pi.restrictions.max_count != "" || pi.restrictions.set_count.size() > 0 ||
        pi.restrictions.category != "" || pi.restrictions.ids.size() > 0 || pi.restrictions.max_length != "")
    {
        Json::Value restrictions;
        if (pi.restrictions.min != "")
            restrictions["MIN"] = cp1251_to_utf8(pi.restrictions.min);
        if (pi.restrictions.max != "")
            restrictions["MAX"] = cp1251_to_utf8(pi.restrictions.max);
        if (pi.restrictions.set_.size() > 0)
        {
            Json::Value set_(Json::arrayValue);
            for (const auto& v : pi.restrictions.set_)
                set_.append(cp1251_to_utf8(v));
            restrictions["SET"] = set_;
        }
        if (pi.restrictions.min_count != "")
            restrictions["MIN_COUNT"] = cp1251_to_utf8(pi.restrictions.min_count);
        if (pi.restrictions.max_count != "")
            restrictions["MAX_COUNT"] = cp1251_to_utf8(pi.restrictions.max_count);
        if (pi.restrictions.set_count.size() > 0)
        {
            Json::Value set_count(Json::arrayValue);
            for (const auto& v : pi.restrictions.set_count)
                set_count.append(cp1251_to_utf8(v));
            restrictions["SET_COUNT"] = set_count;
        }
        if (pi.restrictions.category != "")
            restrictions["CATEGORY"] = cp1251_to_utf8(pi.restrictions.category);
        if (pi.restrictions.ids.size() > 0)
        {
            Json::Value ids(Json::arrayValue);
            for (const auto& v : pi.restrictions.ids)
                ids.append(cp1251_to_utf8(v));
            restrictions["IDS"] = ids;
        }
        if (pi.restrictions.max_length != "")
            restrictions["MAX_LENGTH"] = cp1251_to_utf8(pi.restrictions.max_length);
        info["RESTRICTIONS"] = restrictions;
    }
    emitter.append(info);

    return true;
}
