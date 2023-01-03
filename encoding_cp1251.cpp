#include "encoding_cp1251.h"
#include <string.h>

namespace EncodingCP1251
{
	std::string cp1251_to_utf8( const char *str )
	{
		static const long utf[ 256 ] =
		{
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
			31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,
			59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,
			87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,
			111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,1026,1027,8218,
			1107,8222,8230,8224,8225,8364,8240,1033,8249,1034,1036,1035,1039,1106,8216,8217,
			8220,8221,8226,8211,8212,8250,8482,1113,8250,1114,1116,1115,1119,160,1038,1118,1032,
			164,1168,166,167,1025,169,1028,171,172,173,174,1031,176,177,1030,1110,1169,181,182,
			183,1105,8470,1108,187,1112,1029,1109,1111,1040,1041,1042,1043,1044,1045,1046,1047,
			1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1059,1060,1061,1062,1063,
			1064,1065,1066,1067,1068,1069,1070,1071,1072,1073,1074,1075,1076,1077,1078,1079,
			1080,1081,1082,1083,1084,1085,1086,1087,1088,1089,1090,1091,1092,1093,1094,1095,
			1096,1097,1098,1099,1100,1101,1102,1103
		};

		int cnt = strlen( str );
		std::string out_string;

		for( int i = 0; i < cnt; ++i )
		{
			unsigned char val = ( unsigned char ) str[ i ];
			long c = utf[ val ];

			if( c < 0x80 )
			{
				out_string.append( 1, ( char ) c );
			}
			else if( c < 0x800 )
			{
				out_string.append( 1, ( char ) ( c >> 6 | 0xc0 ) );
				out_string.append( 1, ( char ) ( c & 0x3f | 0x80 ) );
			}
			else if( c < 0x10000 )
			{
				out_string.append( 1, ( char ) ( c >> 12 | 0xe0 ) );
				out_string.append( 1, ( char ) ( c >> 6 & 0x3f | 0x80 ) );
				out_string.append( 1, ( char ) ( c & 0x3f | 0x80 ) );
			}
		}

		return out_string;
	}

	std::string cp1251_to_utf8(const std::string& str)
	{
		return cp1251_to_utf8(str.c_str());
	}

	typedef struct ConvLetter
	{
		char    win1251;
		int     unicode;
	}
	Letter;

	static Letter g_letters[] =
	{
		// 		Полный набор соответствия cp1251 и unicode-16  в рамках кодировки utf-8

		{ ( char ) 0x80,	 0x0402 }, // Cyrillic Capital Letter Dje
		{ ( char ) 0x81,	 0x0403 }, // Cyrillic Capital Letter Gje	
		{ ( char ) 0x82,	0x201a }, // Single Low-9 Quotation Mark
		{ ( char ) 0x83,	0x0453 }, // Cyrillic Small Letter Gje
		{ ( char ) 0x84,	0x201e }, // Double Low-9 Quotation Mark
		{ ( char ) 0x85,	0x2026 }, // Horizontal Ellipsis
		{ ( char ) 0x86,	0x2020 }, // Dagger
		{ ( char ) 0x87,	0x2021 }, // Double Dagger
		{ ( char ) 0x88,	0x20ac }, // Euro Sign
		{ ( char ) 0x89,	0x2030 }, // Per Mille Sign
		{ ( char ) 0x8a,	0x0409 }, // Cyrillic Capital Letter Lje
		{ ( char ) 0x8b,	0x2039 }, // Single Left-Pointing Angle Quotation Mark
		{ ( char ) 0x8c,	0x040a }, // Cyrillic Capital Letter Nje
		{ ( char ) 0x8d,	0x040c }, // Cyrillic Capital Letter Kje
		{ ( char ) 0x8e,	0x040b }, // Cyrillic Capital Letter Tshe
		{ ( char ) 0x8f,	0x040f }, // Cyrillic Capital Letter Dzhe
		{ ( char ) 0x90,	0x0452 }, // Cyrillic Small Letter Dje
		{ ( char ) 0x91,	0x2018 }, // Left Single Quotation Mark
		{ ( char ) 0x92,	0x2019 }, // Right Single Quotation Mark
		{ ( char ) 0x93,	0x201c }, // Left Double Quotation Mark
		{ ( char ) 0x94,	0x201d }, // Right Double Quotation Mark
		{ ( char ) 0x95,	0x2022 }, // Bullet
		{ ( char ) 0x96,	0x2013 }, // En Dash
		{ ( char ) 0x97,	0x2014 }, // Em Dash
		{ ( char ) 0x98,	0x0098 }, // 
		{ ( char ) 0x99,	0x2122 }, // Trade Mark Sign
		{ ( char ) 0x9a,	0x0459 }, // Cyrillic Small Letter Lje
		{ ( char ) 0x9b,	0x203a }, // Single Right-Pointing Angle Quotation Mark
		{ ( char ) 0x9c,	0x045a }, // Cyrillic Small Letter Nje
		{ ( char ) 0x9d,	0x045c }, // Cyrillic Small Letter Kje
		{ ( char ) 0x9e,	0x045b }, // Cyrillic Small Letter Tshe
		{ ( char ) 0x9f,	0x045f }, // Cyrillic Small Letter Dzhe
		{ ( char ) 0xa0,	0x00a0 }, // No-Break Space
		{ ( char ) 0xa1,	0x040e }, // Cyrillic Capital Letter Short U
		{ ( char ) 0xa2,	0x045e }, // Cyrillic Small Letter Short U
		{ ( char ) 0xa3,	0x0408 }, // Cyrillic Capital Letter Je
		{ ( char ) 0xa4,	0x00a4 }, // Currency Sign
		{ ( char ) 0xa5,	0x0490 }, // Cyrillic Capital Letter Ghe With Upturn
		{ ( char ) 0xa6,	0x00a6 }, // Broken Bar
		{ ( char ) 0xa7,	0x00a7 }, // Section Sign
		{ ( char ) 0xa8,	0x0401 }, // Cyrillic Capital Letter Io
		{ ( char ) 0xa9,	0x00a9 }, // Copyright Sign
		{ ( char ) 0xaa,	0x0404 }, // Cyrillic Capital Letter Ukrainian Ie
		{ ( char ) 0xab,	0x00ab }, // Left-Pointing Double Angle Quotation Mark
		{ ( char ) 0xac,	0x00ac }, // Not Sign
		{ ( char ) 0xad,	0x00ad }, // Soft Hyphen
		{ ( char ) 0xae,	0x00ae }, // Registered Sign
		{ ( char ) 0xaf,	0x0407 }, // Cyrillic Capital Letter Yi
		{ ( char ) 0xb0,	0x00b0 }, // Degree Sign
		{ ( char ) 0xb1,	0x00b1 }, // Plus-Minus Sign
		{ ( char ) 0xb2,	0x0406 }, // Cyrillic Capital Letter Byelorussian-Ukrainian I
		{ ( char ) 0xb3,	0x0456 }, // Cyrillic Small Letter Byelorussian-Ukrainian I
		{ ( char ) 0xb4,	0x0491 }, // Cyrillic Small Letter Ghe With Upturn
		{ ( char ) 0xb5,	0x00b5 }, // Micro Sign
		{ ( char ) 0xb6,	0x00b6 }, // Pilcrow Sign
		{ ( char ) 0xb7,	0x00b7 }, // Middle Dot
		{ ( char ) 0xb8,	0x0451 }, // Cyrillic Small Letter Io
		{ ( char ) 0xb9,	0x2116 }, // Numero Sign
		{ ( char ) 0xba,	0x0454 }, // Cyrillic Small Letter Ukrainian Ie
		{ ( char ) 0xbb,	0x00bb }, // Right-Pointing Double Angle Quotation Mark
		{ ( char ) 0xbc,	0x0458 }, // Cyrillic Small Letter Je
		{ ( char ) 0xbd,	0x0405 }, // Cyrillic Capital Letter Dze
		{ ( char ) 0xbe,	0x0455 }, // Cyrillic Small Letter Dze
		{ ( char ) 0xbf,	0x0457 }, // Cyrillic Small Letter Yi
	};

	const int LETTER_COUNT = sizeof( g_letters ) / sizeof( Letter );
	std::map< int, char > mapa;

	unsigned const char _0x11110_ = 30;
	unsigned const char _0x1110_ = 14;
	unsigned const char _0x110_ = 6;
	unsigned const char _0x10_ = 2;
	unsigned const char _0x11111_ = 31;
	unsigned const char _0x111111_ = 63;
	unsigned const char _0x1111_ = 15;
	unsigned const char _0x111_ = 7;

	void FillMapa()
	{
		if( mapa.size() )
		{
			return;
		}

		for( int k = 0; k < LETTER_COUNT; ++k )
		{
			mapa.insert( std::map< int, char >::value_type( g_letters[ k ].unicode, g_letters[ k ].win1251 ) );
		}
	}

	void AssignSymbol( int utf_symbol, char* w1251 )
	{
		std::map< int, char >::iterator f1 = mapa.find( utf_symbol );

		if( mapa.end() != f1 )
		{
			*w1251 = f1->second;
		}
		else
		{
			*w1251 = '?';
		}
	}
	
	std::string utf8_to_cp1251( const char* utf8, unsigned int utf8_sz )
	{
		//**************************************************		
	std::string str_cp1251;
	str_cp1251.reserve(utf8_sz * 4);
	//**************************************************

	FillMapa();

	unsigned j = 0;

	for (unsigned i = 0; i < utf8_sz; ++i)
	{
		unsigned char octet_0 = utf8[i];

		if (0 == (octet_0 >> 7)) // ANSII
		{
			str_cp1251.push_back( octet_0 );
			// str_cp1251[j] = octet_0;
			j += 1;
			continue;
		}

		if (i + 1 >= utf8_sz)
		{
			return std::string();
		}

		unsigned char octet_1 = utf8[i + 1];

		int part0 = 0;
		int part1 = 0;
		int utf_symbol = 0;

		if (_0x110_ == (octet_0 >> 5)) // UTF-8 двухбайтовая кодировка текущего символа
		{
			if (_0x10_ != (octet_1 >> 6))  // признак последовательности многобайтной кодировки
			{
				return std::string();
			}

			part0 = (int)(octet_0 & _0x11111_);
			part0 = part0 << 6;
			part1 = (int)(octet_1 & _0x111111_);
			utf_symbol = part0 + part1;

			if (utf_symbol >= 0x410 && utf_symbol <= 0x44F)
			{
				str_cp1251.push_back((char)(utf_symbol - 0x350));
			}
			else if (utf_symbol >= 0x80 && utf_symbol <= 0xFF)
			{
				str_cp1251.push_back((char)utf_symbol);
			}
			else if (utf_symbol >= 0x402 && utf_symbol <= 0x403)
			{
				str_cp1251.push_back((char)(utf_symbol - 0x382));
			}
			else
			{
				str_cp1251.push_back('?');
				AssignSymbol(utf_symbol, &str_cp1251[j]);
			}

			i += 1;
			j += 1;
			continue;
		}

		if (i + 2 >= utf8_sz)
		{
			return std::string();
		}

		unsigned char octet_2 = utf8[i + 2];
		int part2 = 0;

		if (_0x1110_ == (octet_0 >> 4)) // UTF-8 двухбайтовая кодировка текущего символа
		{
			if (_0x10_ != (octet_1 >> 6))  // признак последовательности многобайтной кодировки
			{
				return std::string();
			}

			if (_0x10_ != (octet_2 >> 6))  // признак последовательности многобайтной кодировки
			{
				return std::string();
			}

			part0 = (int)(octet_0 & _0x1111_);
			part0 = part0 << 12;
			part1 = (int)(octet_1 & _0x111111_);
			part1 = part1 << 6;
			part2 = (int)(octet_2 & _0x111111_);
			utf_symbol = part0 + part1 + part2;

			str_cp1251.push_back('?');
			AssignSymbol(utf_symbol, &str_cp1251[j]);

			i += 2;
			j += 1;
			continue;
		}

		if (i + 3 >= utf8_sz)
		{
			return std::string();
		}

		unsigned char octet_3 = utf8[i + 3];
		int part3 = 0;

		if (_0x11110_ == (octet_0 >> 3)) // UTF-8 двухбайтовая кодировка текущего символа
		{
			if (_0x10_ != (octet_1 >> 6))  // признак последовательности многобайтной кодировки
			{
				return std::string();
			}

			if (_0x10_ != (octet_2 >> 6))  // признак последовательности многобайтной кодировки
			{
				return std::string();
			}

			if (_0x10_ != (octet_3 >> 6))  // признак последовательности многобайтной кодировки
			{
				return std::string();
			}

			part0 = (int)(octet_0 & _0x111_);
			part0 = part0 << 18;
			part1 = (int)(octet_1 & _0x111111_);
			part1 = part1 << 12;
			part2 = (int)(octet_2 & _0x111111_);
			part2 = part2 << 6;
			part3 = (int)(octet_3 & _0x111111_);
			utf_symbol = part0 + part1 + part2 + part3;

			str_cp1251.push_back('?');
			AssignSymbol(utf_symbol, &str_cp1251[j]);

			i += 3;
			j += 1;
			continue;
		}
	}

	return str_cp1251;
	}
	std::string utf8_to_cp1251(const std::string& utf8)
	{
		return utf8_to_cp1251(utf8.c_str(), utf8.size());
	}
}