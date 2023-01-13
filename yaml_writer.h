#ifndef YAML_WRITER_H_
#define YAML_WRITER_H_

#include <string>
#include <vector>
#include "yaml_parser_types.h"
#include "yaml-cpp/yaml.h"

namespace yaml
{
	class writer
	{
	public:
		static bool write(const std::string& filename, file_info& fi);

	private:
		static bool write_file_info(YAML::Emitter& emitter, const file_info& fi);
		static bool write_info_info(YAML::Emitter& emitter, const info_info& ii);
		static bool write_type_info(YAML::Emitter& emitter, const type_info& ti);
		static bool write_parameter_info(YAML::Emitter& emitter, const parameter_info& pi);
	};
}

#endif // YAML_WRITER_H_
