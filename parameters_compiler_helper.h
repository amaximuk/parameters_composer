#ifndef PARAMETERS_COMPILER_HELPER_H_
#define PARAMETERS_COMPILER_HELPER_H_

#include <string>
#include <vector>
#include "parameters_compiler_types.h"

namespace parameters_compiler
{

	class helper
	{
	public:
        static std::string get_hint_html(const struct_types type, const std::string& name)
        {
            if (type == struct_types::file_info)
            {
                if (name == "FILE_FORMAT")
                    return "<p>Номер версии формата yml файла, для обратной совместимости, в данный существует только одна версия.</p><p>Поле не обязательное, если не задано, будет использовано значение 1.</p>";
                else if (name == "INFO")
                    return "<p>Общее описание юнита.</p><p>Обязательная секция.</p>";
                else if (name == "TYPES")
                    return "<p>Пользовательские типы данных для использования в массивах..</p><p>Секция может отсутствовать, если пользовательские типы не используются.</p>";
                else if (name == "PARAMETERS")
                    return "<p>Параметры юнита.</p><p>Секция может отсутствовать, если у юнита нет параметров.</p>";
            }
            else if (type == struct_types::info_info)
            {
                if (name == "ID")
                    return "<p>Идентификатор юнита (имя библиотеки), совпадает с ID в xml файле.</p><p>Обязательное поле.</p>";
                else if (name == "DISPLAY_NAME")
                    return "<p>Имя юнита для отображения, должно быть достаточно кратким, на русском языке.</p><p>Поле не обязательное, если не задано, в качестве имени берется ID<p>";
                else if (name == "DESCRIPTION")
                    return "<p>Текстовое описание назначения юнита, на русском языке. Может содержать несколько строк.</p><p>Поле не обязательное.<p>";
                else if (name == "CATEGORY")
                    return "<p>Категория юнита из матрешки (Handlers, Drivers…).</p><p>Поле не обязательное.<p>";
                else if (name == "HINT")
                    return "<p>Подсказка на типичное значение имени экземпляра данного юнита.</p><p>Поле не обязательное.<p>";
                else if (name == "PICTOGRAM")
                    return "<p>Иконка, закодированная в MIME/BASE64, для отображения в редакторе конфигов.</p><p>Поле не обязательное, в данный момент не используется.<p>";
                else if (name == "AUTHOR")
                    return "<p>Имя автора.</p><p>Поле не обязательное.<p>";
                else if (name == "WIKI")
                    return "<p>Адрес описания юнита в нашей wiki.</p><p>Поле не обязательное.<p>";
            }
            else if (type == struct_types::type_info)
            {
                if (name == "NAME")
                    return "<p>Имя пользовательского типа, при компиляции имя типа остается без изменений и используется для наименования класса параметров и переменных.</p><p>Обязательное поле.</p>";
                else if (name == "TYPE")
                    return "<p>Тип пользовательского типа, yml или enum.</p><p>Поле не обязательное, по умолчанию используется тип yml.<p>";
                else if (name == "DESCRIPTION")
                    return "<p>Текстовое описание типа, на русском языке. Может содержать несколько строк.</p><p>Поле не обязательное.<p>";
                else if (name == "PARAMETERS")
                    return "<p>Параметры, содержащиеся в данном типе. Формат такой же как в основной секции параметров.</p><p>Поле обязательное для пользовательских типов yml.<p>";
                else if (name == "VALUES")
                    return "<p>Допустимые значения для перечисления и их эквивалент в файле cpp. Содержит список пар значений.</p><p>Поле обязательное для пользовательских типов enum.<p>";
                else if (name == "INCLUDES")
                    return "<p>Дополнительно подключаемые заголовочные файлы, необходимые для данного типа.</p><p>Поле не обязательное.<p>";
            }
            else if (type == struct_types::parameter_info)
            {
                if (name == "NAME")
                    return "<p>Имя параметра в xml файле.</p><p>Обязательное поле.</p>";
                else if (name == "TYPE")
                    return "<p>Реальный тип параметра.</p><p>Обязательное поле.<p>";
                else if (name == "DISPLAY_NAME")
                    return "<p>Имя параметра для отображения, должно быть достаточно кратким, на русском языке.</p><p>Поле не обязательное, если не задано, в качестве имени берется NAME.<p>";
                else if (name == "DESCRIPTION")
                    return "<p>Текстовое описание параметра, на русском языке. Может содержать несколько строк.</p><p>Поле не обязательное.<p>";
                else if (name == "REQUIRED")
                    return "<p>Является ли параметр обязательным.</p><p>Поле не обязательное, если не задано, считается равным true.<p>";
                else if (name == "DEFAULT")
                    return "<p>Значение по умолчанию для необязательных параметров.</p><p>Поле не обязательное. Для обязательных параметров не используется. При отсутствии параметра в xml, берется значение по умолчанию, если оно задано, иначе, считаем параметр опциональным.<p>";
                else if (name == "HINT")
                    return "<p>Подсказка на типичное значение параметра.</p><p>Поле не обязательное.<p>";
                else if (name == "RESTRICTIONS")
                    return "<p>Ограничения на параметр (как на значение, так и на количество параметров для массивов).</p><p>Поле не обязательное.<p>";
            }
            else if (type == struct_types::restrictions_info)
            {
                if (name == "MIN")
                    return "<p>Минимальное значение.</p><p>Поле не обязательное.</p>";
                else if (name == "MAX")
                    return "<p>Максимальное значение.</p><p>Поле не обязательное.</p>";
                else if (name == "SET")
                    return "<p>Допустимые значения в виде списка.</p><p>Поле не обязательное.</p>";
                else if (name == "MIN_COUNT")
                    return "<p>Минимальное количество элементов массива.</p><p>Поле не обязательное.</p>";
                else if (name == "MAX_COUNT")
                    return "<p>Максимальное количество элементов массива.</p><p>Поле не обязательное.</p>";
                else if (name == "SET_COUNT")
                    return "<p>Допустимые значения количества элементов массива в виде списка.</p><p>Поле не обязательное.</p>";
                else if (name == "CATEGORY")
                    return "<p>Категория юнитов, которые можно использовать для параметра типа unit.</p><p>Поле не обязательное.</p>";
                else if (name == "IDS")
                    return "<p>Конкретные типы (id) юнитов, которые можно использовать для параметра типа unit, в виде списка.</p><p>Поле не обязательное.</p>";
                else if (name == "MAX_LENGTH")
                    return "<p>Максимальная допустимая длина пути для параметров типа path.</p><p>Поле не обязательное.</p>";
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
                message = "Поле ID является обязательным (Main)";
                return false;
            }

            for (const auto& p : fi.parameters)
            {
                if (p.name == "")
                {
                    message = "Поле NAME у параметров является обязательным (Main)";
                    return false;
                }

                if (p.type == "") // add list of values !!!
                {
                    message = "Поле TYPE у параметров является обязательным (Main/" + p.name + ")";
                    return false;
                }
            }

            for (const auto& t : fi.types)
            {
                if (t.name == "")
                {
                    message = "Поле NAME у типов является обязательным (" + t.name + ")";
                    return false;
                }

                for (const auto& tv : t.parameters)
                {
                    if (tv.name == "")
                    {
                        message = "Поле NAME у параметров является обязательным (" + t.name + ")";
                        return false;
                    }

                    if (tv.type == "") // add list of values !!!
                    {
                        message = "Поле TYPE у параметров является обязательным (" + t.name + "/" + tv.name + ")";
                        return false;
                    }
                }
            }

            return true;
        }

        static bool rearrange_types(file_info& fi)
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
