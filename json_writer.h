#ifndef JSON_WRITER_H_
#define JSON_WRITER_H_

#include <string>
#include <vector>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include "parameters_compiler_types.h"

namespace json
{
	class writer
	{
	public:
		static bool write(const std::string& filename, parameters_compiler::file_info& fi);

	private:
		static bool write_file_info(QJsonObject& emitter, const parameters_compiler::file_info& fi);
		static bool write_info_info(QJsonObject& emitter, const parameters_compiler::info_info& ii);
		static bool write_type_info(QJsonArray& emitter, const parameters_compiler::type_info& ti);
		static bool write_parameter_info(QJsonArray& emitter, const parameters_compiler::parameter_info& pi);
	};
}

#endif // JSON_WRITER_H_
