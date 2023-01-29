#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <regex>
//#include "base64.h"
//#include "git_commit_hash.h"
#include "json_writer.h"

using namespace json;

// !!! remove -> last error???
#define ELRF(message) do { std::cout << message << std::endl; return false; } while(0)

bool writer::write(const std::string& filename, parameters_compiler::file_info& fi)
{
    QJsonObject mainObject;
	if (!write_file_info(mainObject, fi))
		ELRF("File info emit failed");

	std::ofstream of(filename);
	if (!of.is_open())
		ELRF("File " << filename << " open failed");

    // lastly we created a JSON document and set mainObject as object of document
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);

    // Write our jsondocument as json with JSON format
    QByteArray ba = jsonDoc.toJson();
	of.write(ba.data(), ba.size());
	of.close();

	return true;
}

bool writer::write_file_info(QJsonObject& emitter, const parameters_compiler::file_info& fi)
{
    write_info_info(emitter, fi.info);
    if (fi.types.size() > 0)
    {
        QJsonArray types;
        for (const auto& ti : fi.types)
            write_type_info(types, ti);
        emitter.insert("TYPES", types);
    }
    if (fi.parameters.size() > 0)
    {
        QJsonArray parameters;
        for (const auto& pi : fi.parameters)
            write_parameter_info(parameters, pi);
        emitter.insert("PARAMETERS", parameters);
    }

    return true;
}

bool writer::write_info_info(QJsonObject& emitter, const parameters_compiler::info_info& ii)
{
    QJsonObject info;
    info.insert("ID", QString::fromStdString(ii.id));
    if (ii.display_name != "")
        info.insert("DISPLAY_NAME", QString::fromStdString(ii.display_name));
    if (ii.category != "")
        info.insert("CATEGORY", QString::fromStdString(ii.category));
    if (ii.description != "")
        info.insert("DESCRIPTION", QString::fromStdString(ii.description));
    if (ii.pictogram != "")
        info.insert("PICTOGRAM", QString::fromStdString(ii.pictogram));
    if (ii.hint != "")
        info.insert("HINT", QString::fromStdString(ii.hint));
    if (ii.author != "")
        info.insert("AUTHOR", QString::fromStdString(ii.author));
    if (ii.wiki != "")
        info.insert("WIKI", QString::fromStdString(ii.wiki));
    emitter.insert("INFO", info);

    return true;
}

bool writer::write_type_info(QJsonArray& emitter, const parameters_compiler::type_info& ti)
{
    QJsonObject info;
    info.insert("NAME", QString::fromStdString(ti.name));
    if (ti.type != "" && ti.type != "yml")
        info.insert("TYPE", QString::fromStdString(ti.type));
    if (ti.description != "")
        info.insert("DESCRIPTION", QString::fromStdString(ti.description));
    if (ti.values.size() > 0)
    {
        QJsonObject values;
        for (const auto& v : ti.values)
            values.insert(QString::fromStdString(v.first), QString::fromStdString(v.second));
        info.insert("VALUES", values);
    }
    if (ti.includes.size() > 0)
    {
        QJsonArray includes;
        for (const auto& v : ti.includes)
            includes.push_back(QString::fromStdString(v));
        info.insert("INCLUDES", includes);
    }
    if (ti.parameters.size() > 0)
    {
        QJsonArray parameters;
        for (const auto& pi : ti.parameters)
            write_parameter_info(parameters, pi);
        info.insert("PARAMETERS", parameters);
    }
    emitter.push_back(info);
    
    return true;
}

bool writer::write_parameter_info(QJsonArray& emitter, const parameters_compiler::parameter_info& pi)
{
    QJsonObject info;
    info.insert("NAME", QString::fromStdString(pi.name));
    info.insert("TYPE", QString::fromStdString(pi.type));
    if (pi.display_name != "")
        info.insert("DISPLAY_NAME", QString::fromStdString(pi.display_name));
    if (pi.description != "")
        info.insert("DESCRIPTION", QString::fromStdString(pi.description));
    if (pi.required != true)
        info.insert("REQUIRED", pi.required);
    if (pi.default_ != "")
        info.insert("DEFAULT", QString::fromStdString(pi.default_));
    if (pi.hint != "")
        info.insert("HINT", QString::fromStdString(pi.hint));

    if (pi.restrictions.min != "" || pi.restrictions.max != "" || pi.restrictions.set_.size() > 0 ||
        pi.restrictions.min_count != "" || pi.restrictions.max_count != "" || pi.restrictions.set_count.size() > 0 ||
        pi.restrictions.category != "" || pi.restrictions.ids.size() > 0 || pi.restrictions.max_length != "")
    {
        QJsonObject restrictions;
        if (pi.restrictions.min != "")
            restrictions.insert("MIN", QString::fromStdString(pi.restrictions.min));
        if (pi.restrictions.max != "")
            restrictions.insert("MAX", QString::fromStdString(pi.restrictions.max));
        if (pi.restrictions.set_.size() > 0)
        {
            QJsonArray set_;
            for (const auto& v : pi.restrictions.set_)
                set_.push_back(QString::fromStdString(v));
            restrictions.insert("SET", set_);
        }
        if (pi.restrictions.min_count != "")
            restrictions.insert("MIN_COUNT", QString::fromStdString(pi.restrictions.min_count));
        if (pi.restrictions.max_count != "")
            restrictions.insert("MAX_COUNT", QString::fromStdString(pi.restrictions.max_count));
        if (pi.restrictions.set_count.size() > 0)
        {
            QJsonArray set_count;
            for (const auto& v : pi.restrictions.set_count)
                set_count.push_back(QString::fromStdString(v));
            restrictions.insert("SET_COUNT", set_count);
        }
        if (pi.restrictions.category != "")
            restrictions.insert("CATEGORY", QString::fromStdString(pi.restrictions.category));
        if (pi.restrictions.ids.size() > 0)
        {
            QJsonArray ids;
            for (const auto& v : pi.restrictions.ids)
                ids.push_back(QString::fromStdString(v));
            restrictions.insert("IDS", ids);
        }
        if (pi.restrictions.max_length != "")
            restrictions.insert("MAX_LENGTH", QString::fromStdString(pi.restrictions.max_length));
        info.insert("RESTRICTIONS", restrictions);
    }
    emitter.push_back(info);

    return true;
}
