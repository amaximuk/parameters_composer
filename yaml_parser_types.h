#pragma once

#include "encoding_cp1251.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <regex>

namespace yaml
{
	enum class type_category
	{
		system,
		cpp,
		user_yml,
		user_cpp
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
		struct
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
		} yml;

		// Evaluated members ('*' - must be set via TYPE_XML, '-' - not applicable, type == yml.type)
		std::string type;              // unit           double      USER_CPP_ENUM    USER_YML_TYPE    array<unit>                 array<double>          array<USER_CPP_ENUM>          array<USER_YML_TYPE>
		std::string item_type;         // unit           double      USER_CPP_ENUM    USER_YML_TYPE    unit                        double                 USER_CPP_ENUM                 USER_YML_TYPE
		std::string type_xml;          // str            dbl         int*             -                array<str>                  array<dbl>             array<int>*                   array<USER_YML_TYPE>
		std::string item_type_xml;     // str            dbl         int*             -                str                         dbl                    int*                          USER_YML_TYPE
		std::string type_class;        // std::string    double      USER_CPP_ENUM    USER_YML_TYPE    std::vector<std::string>    std::vector<double>    std::vector<USER_CPP_ENUM>    std::vector<USER_YML_TYPE>
		std::string item_type_class;   // std::string    double      USER_CPP_ENUM    USER_YML_TYPE    std::string                 double                 USER_CPP_ENUM                 USER_YML_TYPE
		std::string item_type_get;     // std::string    double      int*             -                std::string                 double                 int*                          -
		bool is_array;
		bool is_required;
		bool is_optional;
		bool has_restrictions;
		enum type_category category;

		inline std::string get_description_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.description);
		}

		inline std::string get_display_name_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.display_name);
		}

		inline std::string get_default_cp1251() const
		{
			if (yml.default_ != "")
				return EncodingCP1251::utf8_to_cp1251(yml.default_);
			return "";
		}

		inline std::string get_default_wiki() const
		{
			return get_default_cp1251();
		}

		inline std::string get_description_wiki() const
		{
			std::string description_wiki = get_description_cp1251();
			description_wiki = std::regex_replace(description_wiki, std::regex("\r\n$|\n$"), "");
			description_wiki = std::regex_replace(description_wiki, std::regex("\r\n|\n"), "<br>");
			return description_wiki;
		}

		inline std::string get_hint_cp1251() const
		{
			if (yml.hint != "")
				return EncodingCP1251::utf8_to_cp1251(yml.hint);
			return "";
		}

		inline std::string get_type_xml_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(type_xml);
		}

		inline std::string get_type_html() const
		{
			std::string type_xml_html = yml.type;
			type_xml_html = std::regex_replace(type_xml_html, std::regex("<"), "&lt;");
			type_xml_html = std::regex_replace(type_xml_html, std::regex(">"), "&gt;");
			return type_xml_html;
		}

		inline std::string get_type_xml_html() const
		{
			std::string type_xml_html = get_type_xml_cp1251();
			type_xml_html = std::regex_replace(type_xml_html, std::regex("<"), "&lt;");
			type_xml_html = std::regex_replace(type_xml_html, std::regex(">"), "&gt;");
			return type_xml_html;
		}

		inline std::string get_type_wiki() const
		{
			return get_type_html();
		}

		inline std::string get_type_xml_wiki() const
		{
			return get_type_xml_html();
		}

		inline std::string get_is_required_cp1251() const
		{
			return is_required ? "да" : "нет";
		}

		inline std::string get_is_required_wiki() const
		{
			return get_is_required_cp1251();
		}

		inline int get_restricted_count_xml() const
		{
			if (has_restrictions && yml.restrictions.min_count != "")
				return atoi(yml.restrictions.min_count.c_str());
			if (has_restrictions && yml.restrictions.max_count != "")
				return atoi(yml.restrictions.max_count.c_str());
			if (has_restrictions && yml.restrictions.set_count.size() > 0)
				return atoi(yml.restrictions.set_count[0].c_str());
			return 0;
		}

		inline std::string get_restricted_value_xml() const
		{
			if (yml.hint != "")
				return get_hint_cp1251();
			if (yml.default_ != "")
				return get_default_cp1251();
			if (has_restrictions && yml.restrictions.min != "")
				return yml.restrictions.min;
			if (has_restrictions && yml.restrictions.max != "")
				return yml.restrictions.max;
			if (has_restrictions && yml.restrictions.set_.size() > 0)
				return EncodingCP1251::utf8_to_cp1251(yml.restrictions.set_[0]);
			if (item_type_xml == "dbl")
				return "0.0";
			if (item_type_xml == "int")
				return "0";
			if (item_type_xml == "bool")
				return "false";
			if (item_type_xml == "str")
				return "Some value";
			return "";
		}

		inline std::vector<std::string> get_restrictions_set_cp1251() const
		{
			std::vector<std::string> set_cp1251;
			for (const auto& s : yml.restrictions.set_)
				set_cp1251.push_back(EncodingCP1251::utf8_to_cp1251(s));
			return set_cp1251;
		}

		inline bool is_user_type() const
		{
			return (category == type_category::user_cpp) || (category == type_category::user_yml);
		}

		inline std::string get_name_text() const
		{
			std::string name_text = yml.name;
			std::transform(name_text.begin(), name_text.end(), name_text.begin(),
				[](unsigned char c) { return std::tolower(c); });
			return name_text;
		}

		inline std::string get_optional_name_text() const
		{
			return get_name_text() + "_is_set";
		}

		inline std::string get_name_name_text() const
		{
			return get_name_text() + "_parameter_name";
		}

		inline std::string get_value_name_text() const
		{
			return get_name_text() + "_parameter_value";
		}

		inline std::string get_count_name_name_text() const
		{
			return get_name_text() + "_parameters_count_name";
		}

		inline std::string get_count_value_name_text() const
		{
			return get_name_text() + "_parameter_count_value";
		}

		inline std::string get_count_xml_name_text() const
		{
			return yml.name + "__CNT";
		}

		inline std::string get_value_set_name_text() const
		{
			return get_value_name_text() + "_set";
		}

		inline std::string get_count_value_set_name_text() const
		{
			return get_count_value_name_text() + "_set";
		}
	};

	struct type_info
	{
		struct
		{
			// Required members from yml
			std::string name;
			// Optional members from yml
			std::string type;
			std::string description;
			std::vector<std::pair<std::string, std::string>> values;
			std::vector<std::string> includes;
		} yml;

		// Evaluated members
		enum type_category category;
		std::vector<parameter_info> parameters;
		std::string type_xml;
		std::string type_get;

		inline std::string get_description_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.description);
		}

		inline std::string get_description_wiki() const
		{
			std::string description_wiki = get_description_cp1251();
			description_wiki = std::regex_replace(description_wiki, std::regex("\r\n$|\n$"), "");
			description_wiki = std::regex_replace(description_wiki, std::regex("\r\n|\n"), "<br>");
			return description_wiki;
		}

		inline bool is_user_yml() const
		{
			return (category == type_category::user_yml);
		}

		inline bool is_user_cpp() const
		{
			return (category == type_category::user_cpp);
		}

		inline std::string get_convert_function_name_text() const
		{
			std::string function_name = yml.name;
			std::transform(function_name.begin(), function_name.end(), function_name.begin(),
				[](unsigned char c) { return std::tolower(c); });
			function_name = std::regex_replace(function_name, std::regex("::"), "_");
			
			return "convert_" + function_name;
		}
	};

	struct info_info
	{
		struct
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
		} yml;

		inline std::string get_display_name_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.display_name);
		}

		inline std::string get_description_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.description);
		}

		inline std::string get_description_wiki() const
		{
			std::string description_wiki = get_description_cp1251();
			description_wiki = std::regex_replace(description_wiki, std::regex("\r\n$|\n$"), "");
			description_wiki = std::regex_replace(description_wiki, std::regex("\r\n|\n"), "<br>");
			return description_wiki;
		}

		inline std::string get_description_html_variables() const
		{
			std::string description_html_variables = get_description_cp1251();
			description_html_variables = std::regex_replace(description_html_variables, std::regex("\r\n$|\n$"), "");
			description_html_variables = std::regex_replace(description_html_variables, std::regex("\r\n|\n"), " ");
			return description_html_variables;
		}

		inline std::string get_author_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.author);
		}

		inline std::string get_wiki_cp1251() const
		{
			return EncodingCP1251::utf8_to_cp1251(yml.wiki);
		}
	};

	struct file_info
	{
		struct
		{
			// Optional members from yml
			std::string format;
		} yml;

		int format;
		info_info info;
		std::vector<type_info> types;
		std::vector<parameter_info> parameters;

		inline bool get_type(const std::string& name, type_info& type) const
		{
			for (const auto& ti : types)
			{
				if (ti.yml.name == name)
				{
					type = ti;
					return true;
				}
			}
			return false;
		}
	};
}