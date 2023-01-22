#ifndef YAML_HELPER_H_
#define YAML_HELPER_H_

#include <string>
#include <vector>
#include <QDebug>
#include "yaml_parser_types.h"
#include "yaml-cpp/yaml.h"

namespace yaml
{
	class helper
	{
	public:
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

        static std::vector<std::string> get_property_type_names(yaml::file_info& fi)
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

		static yaml::type_info* get_type_info(yaml::file_info& fi, const std::string& type)
		{
			const auto it = std::find_if(fi.types.begin(), fi.types.end(), [type](const auto& ti) { return ti.name == type; });
            if (it == fi.types.end())
                return nullptr;
            else
			    return &(*it);
		}

        static bool set_type_info(yaml::file_info& fi, const std::string& type, const yaml::type_info& ti, bool exclude_parameters)
        {
            yaml::type_info* pti = get_type_info(fi, type);
            if (!pti) return false;

            if (exclude_parameters)
            {
                yaml::type_info tit(ti);
                tit.parameters = pti->parameters;
                *pti = tit;
            }
            else
            {
                *pti = ti;
            }
            return true;
        }

		static std::vector<yaml::parameter_info>* get_parameters(yaml::file_info& fi, const std::string& type)
		{
            std::vector<yaml::parameter_info>* pvect = nullptr;
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

		static yaml::parameter_info* get_parameter_info(yaml::file_info& fi, const std::string& type, const std::string& name)
		{
            yaml::parameter_info* ppi = nullptr;
			auto pvect = get_parameters(fi, type);
            if (pvect)
            {
                auto it = std::find_if(pvect->begin(), pvect->end(), [name](const auto& pi) { return pi.name == name; });
                if (it != pvect->end())
                    ppi = &(*it);
            }
			return ppi;
		}

        static bool set_parameter_info(yaml::file_info& fi, const std::string& type, const yaml::parameter_info& pi)
        {
            auto ppi = get_parameter_info(fi, type, pi.name);
            if (!ppi) return false;
            *ppi = pi;
            return true;
        }

        static bool add_parameter_info(yaml::file_info& fi, const std::string& type, const yaml::parameter_info& pi)
        {
            auto ppi = get_parameter_info(fi, type, pi.name);
            if (ppi) return false;
            auto pvect = get_parameters(fi, type);
            if (!pvect) return false;
            pvect->push_back(pi);
            return true;
        }

        static bool remove_parameter_info(yaml::file_info& fi, const std::string& type, const std::string& name)
        {
            auto pvect = get_parameters(fi, type);
            if (!pvect) return false;
            pvect->erase(std::remove_if(pvect->begin(), pvect->end(), [name](const auto& v) { return v.name == name; }), pvect->end());
            return true;
        }

        static bool move_parameter_info(yaml::file_info& fi, const std::string& type, const std::string& name, const bool up)
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

        static bool have_info_value(yaml::file_info& fi, const std::string& type, const std::string& name)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            if (std::find_if(ti->values.cbegin(), ti->values.cend(), [name](const auto& v) {return v.first == name; }) != ti->values.cend())
                return true;
            else
                return false;
        }

        static bool add_info_value(yaml::file_info& fi, const std::string& type, const std::string& name, const std::string& value)
        {
            if (have_info_value(fi, type, name))
                return false;
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            ti->values.push_back(std::make_pair(name, value));
            return true;
        }

        static bool remove_info_value(yaml::file_info& fi, const std::string& type, const std::string& name)
        {
            auto ti = get_type_info(fi, type);
            if (!ti) return false;
            ti->values.erase(std::remove_if(ti->values.begin(), ti->values.end(), [name](const auto& v) { return v.first == name; }), ti->values.end());
            return true;
        }

        static bool move_info_value(yaml::file_info& fi, const std::string& type, const std::string& name, const bool up)
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

        static std::vector<std::string> get_user_type_names(const yaml::file_info& fi)
        {
            std::vector<std::string> result;
            for (const auto& v : fi.types)
                result.push_back(v.name);
            return result;
        }

        static bool validate(yaml::file_info& fi, std::string& message)
        {
            message = "ok";

            if (fi.info.id == "")
            {
                message = "Main ID required";
                return false;
            }

            for (const auto& p : fi.parameters)
            {
                if (p.name == "")
                {
                    message = "Main parameter NAME required";
                    return false;
                }

                if (p.type == "") // add list of values !!!
                {
                    message = "Main parameter TYPE required";
                    return false;
                }
            }

            for (const auto& t : fi.types)
            {
                if (t.name == "")
                {
                    message = "Type NAME required";
                    return false;
                }

                for (const auto& tv : t.parameters)
                {
                    if (tv.name == "")
                    {
                        message = t.name + " type parameter NAME required";
                        return false;
                    }

                    if (tv.type == "") // add list of values !!!
                    {
                        message = t.name + " type parameter TYPE required";
                        return false;
                    }
                }
            }

            return true;
        }

        static bool rearrange_types(yaml::file_info& fi)
        {
            QList<QString> sorted_names;
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

                        if (!yaml::helper::is_inner_type(ts.toStdString()) && !sorted_names.contains(ts))
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
                //QMessageBox::warning(this, "Warning", "Type loop found");
                for (const auto& ti : fi.types)
                {
                    QString tn = QString::fromStdString(ti.name);
                    if (!sorted_names.contains(tn))
                        sorted_names.push_back(tn);
                }
            }

            std::vector<yaml::type_info> sorted_types;
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
	};
}

#endif // YAML_HELPER_H_
