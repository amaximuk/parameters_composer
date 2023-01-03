#ifndef ENCODING_H_
#define ENCODING_H_

#include <string>
#include <map>

namespace EncodingCP1251
{
	std::string cp1251_to_utf8( const char *str );
	std::string cp1251_to_utf8(const std::string & str);
	std::string utf8_to_cp1251( const char* utf8, unsigned int utf8_sz );
	std::string utf8_to_cp1251(const std::string& utf8);
}

#endif // ENCODING_H_
