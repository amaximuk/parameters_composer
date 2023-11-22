#ifndef PARAMETERS_COMPILER_HELPER_H_
#define PARAMETERS_COMPILER_HELPER_H_

#include <string>
#include <vector>
#include "parameters_compiler_types.h"

namespace parameters_compiler
{
    enum class base_types
    {
        none,
        string,
        integer,
        floating,
        bool_,
        array,
        enum_
    };

	class helper
	{
	public:
        static bool is_array_type(const std::string& name)
        {
            QRegularExpression re(R"wwww(^array<(?<value>.*)>)wwww");
            QRegularExpressionMatch match = re.match(QString::fromStdString(name));
            QString type = QString::fromStdString(name);
            bool is_array = false;
            if (match.hasMatch())
            {
                is_array = true;
                type = match.captured("value");
            }
            return is_array;
        }

        static std::string get_array_type(const std::string& name)
        {
            QRegularExpression re(R"wwww(^array<(?<value>.*)>)wwww");
            QRegularExpressionMatch match = re.match(QString::fromStdString(name));
            QString type = QString::fromStdString(name);
            bool is_array = false;
            if (match.hasMatch())
            {
                is_array = true;
                type = match.captured("value");
            }
            return type.toStdString();
        }

        static base_types get_base_type(const std::string& type)
        {
            if (is_array_type(type))
                return base_types::array;
            else if (is_inner_type(type))
            {
                if (type == "unit" || type == "path" || type == "string")
                    return base_types::string;
                else if (type == "int" || type == "int8_t" || type == "int16_t" || type == "int32_t" ||
                    type == "int64_t" || type == "uint8_t" || type == "uint16_t" || type == "uint32_t" || type == "uint64_t")
                    return base_types::integer;
                else if (type == "double" || type == "float")
                    return base_types::floating;
                else if (type == "bool")
                    return base_types::bool_;
                else
                {
                    assert(false);
                    return base_types::none;
                }
            }
            else // enum, yaml must be array
                return base_types::enum_;
        }

        //static std::string get_parameter_hint(const parameters_compiler::parameter_info& pi)
        //{
        //    if (pi.required)
        //        return pi.hint;
        //    else if (pi.default_ != "")
        //        return pi.default_;
        //    else
        //        return pi.hint;
        //}
        static QString get_instance_name_initial(file_info& fi)
        {
            if (fi.info.hint != "")
                return QString::fromStdString(fi.info.hint);

            return QString::fromStdString(fi.info.id);
        }

        static QVariant get_parameter_initial(file_info& fi, const parameter_info& pi, bool is_item)
        {
            QVariant value;
            //bool is_array = is_array_type(pi.type);
            base_types bt = get_base_type(pi.type);
            if (bt == base_types::array && is_item)
                bt = get_base_type(get_array_type(pi.type));

            if (bt == base_types::array)
            {
                if (pi.restrictions.set_count.size() > 0)
                    //value = 0;
                    value = std::stoi(pi.restrictions.set_count[0]);
                else if (pi.restrictions.min_count != "")
                    value = std::stoi(pi.restrictions.min_count);
                else// if (pi.restrictions.max_count != "")
                    value = 0;
            }
            else
            {
                std::string hint;

                if (pi.required)
                    hint = pi.hint;
                else if (pi.default_ != "")
                    hint = pi.default_;
                else
                    hint = pi.hint;

                //if (bt == base_types::string)
                //{
                //    value = QString::fromStdString(hint);
                //}
                //else 
                if (hint == "" && (bt == base_types::enum_ || bt == base_types::integer || bt == base_types::floating))
                {
                    // try get initial from restrictions
                    if (pi.restrictions.set_.size() > 0)
                        hint = pi.restrictions.set_[0];
                    else if (pi.restrictions.min != "" && pi.restrictions.max != "")
                    {
                        if (bt == base_types::enum_)
                            hint = pi.restrictions.min;
                        else if (bt == base_types::integer)
                        {
                            if (std::stoi(pi.restrictions.min) <= 0 && std::stoi(pi.restrictions.max) >= 0)
                                hint = "0";
                            else
                            {
                                int abs_min = std::abs(std::stoi(pi.restrictions.min));
                                int abs_max = std::abs(std::stoi(pi.restrictions.max));
                                hint = abs_min <= abs_max ? pi.restrictions.min : pi.restrictions.max;
                            }
                        }
                        else if (bt == base_types::floating)
                        {
                            if (std::stod(pi.restrictions.min) <= 0 && std::stod(pi.restrictions.max) >= 0)
                                hint = "0";
                            else
                            {
                                double abs_min = std::abs(std::stod(pi.restrictions.min));
                                double abs_max = std::abs(std::stod(pi.restrictions.max));
                                hint = abs_min <= abs_max ? pi.restrictions.min : pi.restrictions.max;
                            }
                        }
                    }
                    else if (pi.restrictions.min != "")
                    {
                        if (bt == base_types::enum_)
                            hint = pi.restrictions.min;
                        else if (bt == base_types::integer)
                        {
                            if (std::stoi(pi.restrictions.min) <= 0)
                                hint = "0";
                            else
                                hint = pi.restrictions.min;
                        }
                        else if (bt == base_types::floating)
                        {
                            if (std::stod(pi.restrictions.min) <= 0)
                                hint = "0";
                            else
                                hint = pi.restrictions.min;
                        }
                    }
                    else if (pi.restrictions.max != "")
                    {
                        if (bt == base_types::enum_)
                            hint = pi.restrictions.min;
                        else if (bt == base_types::integer)
                        {
                            if (std::stod(pi.restrictions.max) >= 0)
                                hint = "0";
                            else
                                hint = pi.restrictions.max;
                        }
                        else if (bt == base_types::floating)
                        {
                            if (std::stod(pi.restrictions.max) >= 0)
                                hint = "0";
                            else
                                hint = pi.restrictions.max;
                        }
                    }
                }

                if (bt == base_types::string)
                {
                    value = QString::fromStdString(hint);
                }
                else if (bt == base_types::bool_)
                {
                    if (hint != "")
                        value = hint == "true" ? true : false;
                    else
                        value = false;
                }
                else if (bt == base_types::enum_)
                {
                    // get index of hint value or 0 (index of first enum element)
                    type_info* ti = get_type_info(fi, pi.type);

                    if (ti != nullptr && ti->type == "enum" && ti->values.size() > 0)
                    {
                        if (hint == "")
                            value = 0;
                        else
                        {
                            auto it = std::find_if(ti->values.cbegin(), ti->values.cend(), [hint](const auto& v) { return v.first == hint; });
                            if (it != ti->values.cend())
                                value = static_cast<int>(std::distance(ti->values.cbegin(), it));
                            else
                                value = 0;
                        }
                    }
                    else
                        value = 0;
                }
                else if (bt == base_types::integer)
                {
                    if (hint != "")
                        value = std::stoi(hint);
                    else
                        value = 0;
                }
                else if (bt == base_types::floating)
                {
                    if (hint != "")
                        value = std::stod(hint);
                    else
                        value = 0.0;
                }
            }

            return value;
        }

        //template<typename T>
        //static T get_parameter_initial(file_info& fi, const parameter_info& pi)
        //{
        //    return {};
        //}

        //template<>
        //static std::string get_parameter_initial(file_info& fi, const parameter_info& pi)
        //{
        //    std::string hint;
        //    bool is_array = is_array_type(pi.type);
        //    if (is_array)
        //    {
        //        if (pi.restrictions.set_count.size() > 0)
        //            hint = pi.restrictions.set_count[0];
        //        else if (pi.restrictions.min_count != "")
        //            hint = pi.restrictions.min_count;
        //        else if (pi.restrictions.max_count != "")
        //            hint = "0";
        //    }
        //    else
        //    {
        //        type_info* ti = get_type_info(fi, pi.type);

        //        if (pi.required)
        //            hint = pi.hint;
        //        else if (pi.default_ != "")
        //            hint = pi.default_;
        //        else
        //            hint = pi.hint;

        //        if (hint == "")
        //        {
        //            if (pi.restrictions.set_.size() > 0)
        //                hint = pi.restrictions.set_[0];
        //            else if (pi.restrictions.min != "" && pi.restrictions.max != "")
        //            {
        //                if (std::stoi(pi.restrictions.min) <= 0 && std::stoi(pi.restrictions.max) >= 0)
        //                    hint = "0";
        //                else
        //                {
        //                    int abs_min = std::abs(std::stoi(pi.restrictions.min));
        //                    int abs_max = std::abs(std::stoi(pi.restrictions.max));
        //                    hint = abs_min <= abs_max ? pi.restrictions.min : pi.restrictions.max;
        //                }
        //            }
        //            else if (pi.restrictions.min != "")
        //            {
        //                if (std::stoi(pi.restrictions.min) <= 0)
        //                    hint = "0";
        //                else
        //                    hint = pi.restrictions.min;
        //            }
        //            else if (pi.restrictions.max != "")
        //            {
        //                if (std::stoi(pi.restrictions.max) >= 0)
        //                    hint = "0";
        //                else
        //                    hint = pi.restrictions.max;
        //            }
        //        }

        //        if (ti != nullptr && ti->type == "enum" && ti->values.size() > 0)
        //        {
        //            if (hint == "")
        //                hint = "0"; // index of first enum element
        //            else
        //            {
        //                auto it = std::find_if(ti->values.cbegin(), ti->values.cend(), [hint](const auto& v) { return v.first == hint; });
        //                if (it != ti->values.cend())
        //                    hint = std::to_string(std::distance(ti->values.cbegin(), it));
        //                else
        //                    hint = "0"; // index of first enum element
        //            }
        //            //hint = ti->values[0].first;
        //        }
        //    }

        //    if (hint == "")
        //    { }

        //    return hint;
        //}

        //template<>
        //static int get_parameter_initial(file_info& fi, const parameter_info& pi)
        //{
        //    return std::stoi(get_parameter_initial<std::string>(fi, pi));
        //}

        //template<>
        //static double get_parameter_initial(file_info& fi, const parameter_info& pi)
        //{
        //    return std::stod(get_parameter_initial<std::string>(fi, pi));
        //}

        //template<>
        //static bool get_parameter_initial(file_info& fi, const parameter_info& pi)
        //{
        //    return get_parameter_initial<std::string>(fi, pi) == "true" ? true : false;
        //}

        static std::string get_parameter_display_name(const parameter_info& pi)
        {
            if (pi.display_name != "")
                return pi.display_name;
            else
                return pi.name;
        }

        static bool get_parameter_optional(const parameter_info& pi)
        {
            if (!pi.required && pi.default_ == "")
                return true;
            else
                return false;
        }

        static std::string get_hint_html(const struct_types type, const std::string& name)
        {
            if (type == struct_types::file_info)
            {
                if (name == "FILE_FORMAT")
                    return "<p>����� ������ ������� yml �����, ��� �������� �������������, � ������ ���������� ������ ���� ������.</p><p>���� �� ������������, ���� �� ������, ����� ������������ �������� 1.</p>";
                else if (name == "INFO")
                    return "<p>����� �������� �����.</p><p>������������ ������.</p>";
                else if (name == "TYPES")
                    return "<p>���������������� ���� ������ ��� ������������� � ��������..</p><p>������ ����� �������������, ���� ���������������� ���� �� ������������.</p>";
                else if (name == "PARAMETERS")
                    return "<p>��������� �����.</p><p>������ ����� �������������, ���� � ����� ��� ����������.</p>";
            }
            else if (type == struct_types::info_info)
            {
                if (name == "ID")
                    return "<p>������������� ����� (��� ����������), ��������� � ID � xml �����.</p><p>������������ ����.</p>";
                else if (name == "DISPLAY_NAME")
                    return "<p>��� ����� ��� �����������, ������ ���� ���������� �������, �� ������� �����.</p><p>���� �� ������������, ���� �� ������, � �������� ����� ������� ID<p>";
                else if (name == "DESCRIPTION")
                    return "<p>��������� �������� ���������� �����, �� ������� �����. ����� ��������� ��������� �����.</p><p>���� �� ������������.<p>";
                else if (name == "CATEGORY")
                    return "<p>��������� ����� �� �������� (Handlers, Drivers�).</p><p>���� �� ������������.<p>";
                else if (name == "HINT")
                    return "<p>��������� �� �������� �������� ����� ���������� ������� �����.</p><p>���� �� ������������.<p>";
                else if (name == "PICTOGRAM")
                    return "<p>������, �������������� � MIME/BASE64, ��� ����������� � ��������� ��������.</p><p>���� �� ������������, � ������ ������ �� ������������.<p>";
                else if (name == "AUTHOR")
                    return "<p>��� ������.</p><p>���� �� ������������.<p>";
                else if (name == "WIKI")
                    return "<p>����� �������� ����� � ����� wiki.</p><p>���� �� ������������.<p>";
            }
            else if (type == struct_types::type_info)
            {
                if (name == "NAME")
                    return "<p>��� ����������������� ����, ��� ���������� ��� ���� �������� ��� ��������� � ������������ ��� ������������ ������ ���������� � ����������.</p><p>������������ ����.</p>";
                else if (name == "TYPE")
                    return "<p>��� ����������������� ����, yml ��� enum.</p><p>���� �� ������������, �� ��������� ������������ ��� yml.<p>";
                else if (name == "DESCRIPTION")
                    return "<p>��������� �������� ����, �� ������� �����. ����� ��������� ��������� �����.</p><p>���� �� ������������.<p>";
                else if (name == "PARAMETERS")
                    return "<p>���������, ������������ � ������ ����. ������ ����� �� ��� � �������� ������ ����������.</p><p>���� ������������ ��� ���������������� ����� yml.<p>";
                else if (name == "VALUES")
                    return "<p>���������� �������� ��� ������������ � �� ���������� � ����� cpp. �������� ������ ��� ��������.</p><p>���� ������������ ��� ���������������� ����� enum.<p>";
                else if (name == "INCLUDES")
                    return "<p>������������� ������������ ������������ �����, ����������� ��� ������� ����.</p><p>���� �� ������������.<p>";
            }
            else if (type == struct_types::parameter_info)
            {
                if (name == "NAME")
                    return "<p>��� ��������� � xml �����.</p><p>������������ ����.</p>";
                else if (name == "TYPE")
                    return "<p>�������� ��� ���������.</p><p>������������ ����.<p>";
                else if (name == "DISPLAY_NAME")
                    return "<p>��� ��������� ��� �����������, ������ ���� ���������� �������, �� ������� �����.</p><p>���� �� ������������, ���� �� ������, � �������� ����� ������� NAME.<p>";
                else if (name == "DESCRIPTION")
                    return "<p>��������� �������� ���������, �� ������� �����. ����� ��������� ��������� �����.</p><p>���� �� ������������.<p>";
                else if (name == "REQUIRED")
                    return "<p>�������� �� �������� ������������.</p><p>���� �� ������������, ���� �� ������, ��������� ������ true.<p>";
                else if (name == "DEFAULT")
                    return "<p>�������� �� ��������� ��� �������������� ����������.</p><p>���� �� ������������. ��� ������������ ���������� �� ������������. ��� ���������� ��������� � xml, ������� �������� �� ���������, ���� ��� ������, �����, ������� �������� ������������.<p>";
                else if (name == "HINT")
                    return "<p>��������� �� �������� �������� ���������.</p><p>���� �� ������������.<p>";
                else if (name == "RESTRICTIONS")
                    return "<p>����������� �� �������� (��� �� ��������, ��� � �� ���������� ���������� ��� ��������).</p><p>���� �� ������������.<p>";
            }
            else if (type == struct_types::restrictions_info)
            {
                if (name == "MIN")
                    return "<p>����������� ��������.</p><p>���� �� ������������.</p>";
                else if (name == "MAX")
                    return "<p>������������ ��������.</p><p>���� �� ������������.</p>";
                else if (name == "SET")
                    return "<p>���������� �������� � ���� ������.</p><p>���� �� ������������.</p>";
                else if (name == "MIN_COUNT")
                    return "<p>����������� ���������� ��������� �������.</p><p>���� �� ������������.</p>";
                else if (name == "MAX_COUNT")
                    return "<p>������������ ���������� ��������� �������.</p><p>���� �� ������������.</p>";
                else if (name == "SET_COUNT")
                    return "<p>���������� �������� ���������� ��������� ������� � ���� ������.</p><p>���� �� ������������.</p>";
                else if (name == "CATEGORY")
                    return "<p>��������� ������, ������� ����� ������������ ��� ��������� ���� unit.</p><p>���� �� ������������.</p>";
                else if (name == "IDS")
                    return "<p>���������� ���� (id) ������, ������� ����� ������������ ��� ��������� ���� unit, � ���� ������.</p><p>���� �� ������������.</p>";
                else if (name == "MAX_LENGTH")
                    return "<p>������������ ���������� ����� ���� ��� ���������� ���� path.</p><p>���� �� ������������.</p>";
            }

            return "";
        }

		static bool is_inner_type(std::string name)
		{
            auto it = std::find_if(parameter_type_names_.cbegin(), parameter_type_names_.cend(), [name](const auto& v) { return v == name; });
            if (it != parameter_type_names_.cend())
                return true;
			return false;
		}

        static std::vector<std::string> get_type_type_names()
        {
            return type_type_names_;
        }

        static std::vector<std::string> get_property_type_names(file_info& fi)
        {
            std::vector<std::string> result;
            for (const auto& v : fi.types)
            {
                // yml types can be used in array only
                if (v.type == "enum")
                    result.push_back(v.name);
                result.push_back("array<" + v.name + ">");
            }
            result.insert(result.end(), parameter_type_names_.cbegin(), parameter_type_names_.cend());
            return result;
        }

        static std::vector<std::string> get_user_type_names(const file_info& fi)
        {
            std::vector<std::string> result;
            for (const auto& v : fi.types)
                result.push_back(v.name);
            return result;
        }

		static type_info* get_type_info(file_info& fi, const std::string& type)
		{
			const auto it = std::find_if(fi.types.begin(), fi.types.end(), [type](const auto& ti) { return ti.name == type; });
            if (it == fi.types.end())
                return nullptr;
            else
			    return &(*it);
		}

        static bool set_type_info(file_info& fi, const std::string& type, const type_info& ti, bool exclude_parameters)
        {
            type_info* pti = get_type_info(fi, type);
            if (!pti) return false;

            if (exclude_parameters)
            {
                type_info tit(ti);
                tit.parameters = pti->parameters;
                *pti = tit;
            }
            else
            {
                *pti = ti;
            }
            return true;
        }

		static std::vector<parameter_info>* get_parameters(file_info& fi, const std::string& type)
		{
            std::vector<parameter_info>* pvect = nullptr;
            if (type == "Main")
            {
                pvect = &fi.parameters;
            }
            else
            {
                auto pti = get_type_info(fi, type);
                if (pti) pvect = &pti->parameters;
            }
            return pvect;
		}

		static parameter_info* get_parameter_info(file_info& fi, const std::string& type, const std::string& name)
		{
            parameter_info* ppi = nullptr;
			auto pvect = get_parameters(fi, type);
            if (pvect)
            {
                auto it = std::find_if(pvect->begin(), pvect->end(), [name](const auto& pi) { return pi.name == name; });
                if (it != pvect->end())
                    ppi = &(*it);
            }
			return ppi;
		}

        static bool extract_array_file_info(file_info& fi, const std::string& type, const std::string& name, file_info& afi)
        {
            afi = {};

            // �������� ������ ��� �������� ����������������� ���� ������, �� ������������
            // ������� ��� �������� �������, ��������� ��� ��� ��������� � �������� ������
            // ��� ����, �������� ���� ����� ����� ��� �� �����,�� �������
            // ��� � �������� ����� �������� �� ��� � �������� ���������

            auto pi = parameters_compiler::helper::get_parameter_info(fi, type, name);
            if (pi == nullptr)
                return false;

            bool is_array = parameters_compiler::helper::is_array_type(pi->type);
            bool is_inner_type = parameters_compiler::helper::is_inner_type(pi->type);

            if (is_array && !is_inner_type)
            {
                std::string array_type = parameters_compiler::helper::get_array_type(pi->type);

                qDebug() << QString::fromStdString(array_type);
                auto pti = get_type_info(fi, array_type);

                if (pti == nullptr || pti->type != "yml")
                    return false;

                afi.format = fi.format;
                afi.info = fi.info;
                afi.info.id = pti->name;
                afi.info.display_name = pi->display_name;
                afi.info.description = pi->description;
                afi.parameters = pti->parameters;

                for (const auto& t : fi.types)
                {
                    if (t.name != array_type)
                        afi.types.push_back(t);
                    else
                        break;
                }
            }
            else
                return false;

            return true;
        }

        static bool set_parameter_info(file_info& fi, const std::string& type, const parameter_info& pi)
        {
            auto ppi = get_parameter_info(fi, type, pi.name);
            if (!ppi) return false;
            *ppi = pi;
            return true;
        }

        static bool add_parameter_info(file_info& fi, const std::string& type, const parameter_info& pi)
        {
            auto ppi = get_parameter_info(fi, type, pi.name);
            if (ppi) return false;
            auto pvect = get_parameters(fi, type);
            if (!pvect) return false;
            pvect->push_back(pi);
            return true;
        }

        static bool remove_parameter_info(file_info& fi, const std::string& type, const std::string& name)
        {
            auto pvect = get_parameters(fi, type);
            if (!pvect) return false;
            pvect->erase(std::remove_if(pvect->begin(), pvect->end(), [name](const auto& v) { return v.name == name; }), pvect->end());
            return true;
        }

        static bool move_parameter_info(file_info& fi, const std::string& type, const std::string& name, const bool up)
        {
            auto pvect = get_parameters(fi, type);
            if (!pvect) return false;
            auto it = std::find_if(pvect->begin(), pvect->end(), [name](auto& p) { return p.name == name; });
            if (up && it != pvect->begin() && it != pvect->end())
                std::iter_swap(it, std::next(it, -1));
            else if(!up && it != std::next(pvect->end(), -1) && it != pvect->end())
                std::iter_swap(it, std::next(it, 1));
            return true;
        }

        static bool have_info_value(file_info& fi, const std::string& type, const std::string& name)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            if (std::find_if(ti->values.cbegin(), ti->values.cend(), [name](const auto& v) {return v.first == name; }) != ti->values.cend())
                return true;
            else
                return false;
        }

        static bool add_info_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            if (have_info_value(fi, type, name))
                return false;
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            ti->values.push_back(std::make_pair(name, value));
            return true;
        }

        static bool remove_info_value(file_info& fi, const std::string& type, const std::string& name)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            ti->values.erase(std::remove_if(ti->values.begin(), ti->values.end(), [name](const auto& v) { return v.first == name; }), ti->values.end());
            return true;
        }

        static bool move_info_value(file_info& fi, const std::string& type, const std::string& name, const bool up)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            auto it = std::find_if(ti->values.begin(), ti->values.end(), [name](const auto& v) { return v.first == name; });
            if (up && it != ti->values.begin() && it != ti->values.end())
                std::iter_swap(it, std::next(it, -1));
            else if (!up && it != std::next(ti->values.end(), -1) && it != ti->values.end())
                std::iter_swap(it, std::next(it, 1));
            return true;
        }

        static bool have_info_include(file_info& fi, const std::string& type, const std::string& name)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            if (std::find_if(ti->includes.cbegin(), ti->includes.cend(), [name](const auto& v) {return v == name; }) != ti->includes.cend())
                return true;
            else
                return false;
        }

        static bool add_info_include(file_info& fi, const std::string& type, const std::string& name)
        {
            if (have_info_include(fi, type, name))
                return false;
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            ti->includes.push_back(name);
            return true;
        }

        static bool remove_info_include(file_info& fi, const std::string& type, const std::string& name)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            ti->includes.erase(std::remove_if(ti->includes.begin(), ti->includes.end(), [name](const auto& v) { return v == name; }), ti->includes.end());
            return true;
        }

        static bool move_info_include(file_info& fi, const std::string& type, const std::string& name, const bool up)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            auto it = std::find_if(ti->includes.begin(), ti->includes.end(), [name](const auto& v) { return v == name; });
            if (up && it != ti->includes.begin() && it != ti->includes.end())
                std::iter_swap(it, std::next(it, -1));
            else if (!up && it != std::next(ti->includes.end(), -1) && it != ti->includes.end())
                std::iter_swap(it, std::next(it, 1));
            return true;
        }

        static bool have_properties_set_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            if (std::find_if(ppi->restrictions.set_.cbegin(), ppi->restrictions.set_.cend(), [value](const auto& v) {return v == value; }) != ppi->restrictions.set_.cend())
                return true;
            else
                return false;
        }

        static bool add_properties_set_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            if (have_properties_set_value(fi, type, name, value))
                return false;
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            ppi->restrictions.set_.push_back(value);
            return true;
        }

        static bool remove_properties_set_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            ppi->restrictions.set_.erase(std::remove_if(ppi->restrictions.set_.begin(), ppi->restrictions.set_.end(), [value](const auto& v) { return v == value; }), ppi->restrictions.set_.end());
            return true;
        }

        static bool move_properties_set_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value, const bool up)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            auto it = std::find_if(ppi->restrictions.set_.begin(), ppi->restrictions.set_.end(), [value](const auto& v) { return v == value; });
            if (up && it != ppi->restrictions.set_.begin() && it != ppi->restrictions.set_.end())
                std::iter_swap(it, std::next(it, -1));
            else if (!up && it != std::next(ppi->restrictions.set_.end(), -1) && it != ppi->restrictions.set_.end())
                std::iter_swap(it, std::next(it, 1));
            return true;
        }

        static bool have_properties_set_count_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            if (std::find_if(ppi->restrictions.set_count.cbegin(), ppi->restrictions.set_count.cend(), [value](const auto& v) {return v == value; }) != ppi->restrictions.set_count.cend())
                return true;
            else
                return false;
        }

        static bool add_properties_set_count_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            if (have_properties_set_count_value(fi, type, name, value))
                return false;
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            ppi->restrictions.set_count.push_back(value);
            return true;
        }

        static bool remove_properties_set_count_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            ppi->restrictions.set_count.erase(std::remove_if(ppi->restrictions.set_count.begin(), ppi->restrictions.set_count.end(), [value](const auto& v) { return v == value; }), ppi->restrictions.set_count.end());
            return true;
        }

        static bool move_properties_set_count_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value, const bool up)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            auto it = std::find_if(ppi->restrictions.set_count.begin(), ppi->restrictions.set_count.end(), [value](const auto& v) { return v == value; });
            if (up && it != ppi->restrictions.set_count.begin() && it != ppi->restrictions.set_count.end())
                std::iter_swap(it, std::next(it, -1));
            else if (!up && it != std::next(ppi->restrictions.set_count.end(), -1) && it != ppi->restrictions.set_count.end())
                std::iter_swap(it, std::next(it, 1));
            return true;
        }

        static bool have_properties_ids_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            if (std::find_if(ppi->restrictions.ids.cbegin(), ppi->restrictions.ids.cend(), [value](const auto& v) {return v == value; }) != ppi->restrictions.ids.cend())
                return true;
            else
                return false;
        }

        static bool add_properties_ids_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            if (have_properties_ids_value(fi, type, name, value))
                return false;
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            ppi->restrictions.ids.push_back(value);
            return true;
        }

        static bool remove_properties_ids_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            ppi->restrictions.ids.erase(std::remove_if(ppi->restrictions.ids.begin(), ppi->restrictions.ids.end(), [value](const auto& v) { return v == value; }), ppi->restrictions.ids.end());
            return true;
        }

        static bool move_properties_ids_value(file_info& fi, const std::string& type, const std::string& name, const std::string& value, const bool up)
        {
            auto ppi = get_parameter_info(fi, type, name);
            if (!ppi) return false;
            auto it = std::find_if(ppi->restrictions.ids.begin(), ppi->restrictions.ids.end(), [value](const auto& v) { return v == value; });
            if (up && it != ppi->restrictions.ids.begin() && it != ppi->restrictions.ids.end())
                std::iter_swap(it, std::next(it, -1));
            else if (!up && it != std::next(ppi->restrictions.ids.end(), -1) && it != ppi->restrictions.ids.end())
                std::iter_swap(it, std::next(it, 1));
            return true;
        }

        static bool validate(file_info& fi, std::string& message)
        {
            message = "ok";

            if (fi.info.id == "")
            {
                message = "���� ID �������� ������������ (Main)";
                return false;
            }

            for (const auto& p : fi.parameters)
            {
                if (p.name == "")
                {
                    message = "���� NAME � ���������� �������� ������������ (Main)";
                    return false;
                }

                if (p.type == "") // add list of values !!!
                {
                    message = "���� TYPE � ���������� �������� ������������ (Main/" + p.name + ")";
                    return false;
                }
            }

            for (const auto& t : fi.types)
            {
                if (t.name == "")
                {
                    message = "���� NAME � ����� �������� ������������ (" + t.name + ")";
                    return false;
                }

                for (const auto& tv : t.parameters)
                {
                    if (tv.name == "")
                    {
                        message = "���� NAME � ���������� �������� ������������ (" + t.name + ")";
                        return false;
                    }

                    if (tv.type == "") // add list of values !!!
                    {
                        message = "���� TYPE � ���������� �������� ������������ (" + t.name + "/" + tv.name + ")";
                        return false;
                    }
                }
            }

            return true;
        }

        static bool rearrange_types(file_info& fi, bool& have_type_loop)
        {
            QList<QString> sorted_names;
            have_type_loop = false;
            bool found_new = true;
            while (found_new)
            {
                found_new = false;
                for (const auto& ti : fi.types)
                {
                    QString tn = QString::fromStdString(ti.name);
                    if (sorted_names.contains(tn))
                        continue;

                    bool have_unresolved = false;
                    for (const auto& pi : ti.parameters)
                    {
                        QString ts = QString::fromStdString(pi.type);
                        if (ts.startsWith("array") && ts.length() > 7)
                            ts = ts.mid(6, ts.length() - 7);

                        if (!helper::is_inner_type(ts.toStdString()) && !sorted_names.contains(ts))
                        {
                            have_unresolved = true;
                            break;
                        }
                    }

                    if (!have_unresolved)
                    {
                        sorted_names.push_back(tn);
                        found_new = true;
                    }
                }
            }

            if (fi.types.size() > sorted_names.size())
            {
                // Type loop found
                have_type_loop = true;
                for (const auto& ti : fi.types)
                {
                    QString tn = QString::fromStdString(ti.name);
                    if (!sorted_names.contains(tn))
                        sorted_names.push_back(tn);
                }
            }

            std::vector<type_info> sorted_types;
            for (const auto& sn : sorted_names)
            {
                for (const auto& ti : fi.types)
                {
                    if (sn == QString::fromStdString(ti.name))
                    {
                        sorted_types.push_back(ti);
                    }
                }
            }

            fi.types = std::move(sorted_types);

            return true;
        }

        static bool rename_type(file_info& fi, const std::string& oldName, const std::string& newName)
        {
            std::string arrayOldName = "array<" + oldName + ">";
            std::string arrayNewName = "array<" + newName + ">";

            for (auto& t : fi.types)
            {
                if (t.name == oldName)
                {
                    t.name = newName;
                    break;
                }
            }

            for (auto& p : fi.parameters)
            {
                if (p.type == oldName)
                {
                    p.type = newName;
                    break;
                }
                if (p.type == arrayOldName)
                {
                    p.type = arrayNewName;
                    break;
                }
            }
            for (auto& t : fi.types)
            {
                for (auto& p : t.parameters)
                {
                    if (p.type == oldName)
                    {
                        p.type = newName;
                        break;
                    }
                    if (p.type == arrayOldName)
                    {
                        p.type = arrayNewName;
                        break;
                    }
                }
            }

            return true;
        }

        static bool rename_property(file_info& fi, const std::string& type, const std::string& oldName, const std::string& newName)
        {
            auto ppi = get_parameter_info(fi, type, oldName);
            if (!ppi) return false;
            ppi->name = newName;
            return true;
        }
	};
}

#endif // PARAMETERS_COMPILER_HELPER_H_
