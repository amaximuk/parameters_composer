#ifndef REFACTORING_HELPER_H_
#define REFACTORING_HELPER_H_

#include <QString>
#include <QFile>
#include <QTextStream>

namespace refactoring
{
	class helper
	{
	public:
        static bool read_file(const QString& fileName, const QString& encoding, QString& text)
        {
            QFile fr(fileName);
            if (!fr.open(QFile::ReadOnly | QFile::Text))
                return false;
            QTextStream in(&fr);
            in.setCodec(encoding.toStdString().c_str());
            text = in.readAll();
            fr.close();

            return true;
        }

        static bool get_single_value(const QString& text, const QString& regexp, QString& value)
        {
            QRegularExpression re(regexp);
            re.setPatternOptions(QRegularExpression::MultilineOption);
            QRegularExpressionMatch match = re.match(text);
            if (!match.hasMatch())
                return false;
            value = match.captured("value");
            return true;
        }

    
        static bool patch_cmake(const QString& text, QString& result)
        {
            QString cmake_text(text);

            int add_library_index = cmake_text.indexOf("add_library");
            if (add_library_index == -1)
                return false;
            int add_library_close_index = cmake_text.indexOf(")", add_library_index);
            if (add_library_close_index == -1)
                return false;

            int add_library_before_index = cmake_text.lastIndexOf("\n\n", add_library_index);
            if (add_library_before_index == -1)
                return false;
            int add_library_after_index = cmake_text.indexOf("\n\n", add_library_index);
            if (add_library_after_index == -1)
            {
                if (cmake_text.endsWith("\n") || cmake_text.endsWith("\n\t"))
                {
                    add_library_after_index = cmake_text.size();
                }
                else
                {
                    add_library_after_index = cmake_text.size() + 1;
                    cmake_text.append("\n");
                }
            }

            // # генерируем файл параметров, заполняем PROJECT_PARAMETERS
            // prepare_compile_parameters()
            // ${PROJECT_PARAMETERS}
            // # подписка для обновления параметров
            // add_compile_parameters(${TARGET_NAME})

            QString result_text;
            QString string;
            int index = 0;

            string = cmake_text.left(add_library_before_index + 1);
            result_text.append(string);
            index = add_library_before_index + 1;

            string = QString::fromLocal8Bit("\n# генерируем файл параметров, заполняем PROJECT_PARAMETERS\n");
            result_text.append(string);

            string = QString::fromLocal8Bit("prepare_compile_parameters()\n");
            result_text.append(string);

            string = cmake_text.mid(index, add_library_close_index - index);
            result_text.append(string);
            index += add_library_close_index - index;

            string = QString::fromLocal8Bit(" ${PROJECT_PARAMETERS})");
            result_text.append(string);
            index += 1; // ")"

            string = cmake_text.mid(index, add_library_after_index - index + 1);
            result_text.append(string);
            index += add_library_after_index - index + 1;

            string = QString::fromLocal8Bit("\n# подписка для обновления параметров\n");
            result_text.append(string);

            string = QString::fromLocal8Bit("add_compile_parameters(${TARGET_NAME})\n");
            result_text.append(string);

            if (cmake_text.length() > index)
            {
                string = cmake_text.right(cmake_text.length() - index);
                result_text.append(string);
            }

            return true;
        }
    };
}

#endif // REFACTORING_HELPER_H_
