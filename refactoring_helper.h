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

        static bool write_file(const QString& fileName, const QString& encoding, const QString& text)
        {
            QFile fw(fileName);
            if (!fw.open(QFile::WriteOnly | QFile::Text))
                return false;
            QTextStream out(&fw);
            out.setCodec("windows-1251");
            out << text;
            fw.close();
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
    
        static bool check_cmake_is_patched(const QString& text)
        {
            if (text.contains("prepare_compile_parameters()"))
                return true;
            else
                return false;
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

            result = result_text;
            return true;
        }

        static bool check_include_is_patched(const QString& text)
        {
            if (text.contains("parameters_compiler::parameters parameters_;"))
                return true;
            else
                return false;
        }

        static bool patch_include(const QString& text, const QString& target_name, const QString& class_name, QString& result)
        {
            QString include_text(text);

            QString result_text;
            QString string;
            int index = 0;

            int class_include_index = include_text.indexOf("#include");
            if (class_include_index == -1)
                return false;
            int class_include_close_index = include_text.indexOf("\n", class_include_index);
            if (class_include_close_index == -1)
                return false;

            QString class_class_name = QString("class(\\s+)%1").arg(class_name);
            int class_class_index = include_text.indexOf(QRegExp(class_class_name));
            if (class_class_index == -1)
                return false;
            class_class_index = include_text.indexOf("{", class_class_index);
            if (class_class_index == -1)
                return false;
            class_class_index++;
            if (include_text.length() <= class_class_index)
                return false;

            string = include_text.left(class_include_close_index);
            result_text.append(string);
            index = class_include_close_index;

            string = QString::fromLocal8Bit("\n#include \"%1.yml.h\"").arg(target_name);
            result_text.append(string);

            string = include_text.mid(index, class_class_index - index + 1);
            result_text.append(string);
            index += class_class_index - index + 1;

            string = QString::fromLocal8Bit("\nprivate:\n\tparameters_compiler::parameters parameters_;\n");
            result_text.append(string);

            if (include_text.length() > class_class_index)
            {
                string = include_text.right(include_text.length() - class_class_index);
                result_text.append(string);
            }

            result = result_text;
            return true;
        }

        static bool check_cpp_is_patched(const QString& text)
        {
            if (text.contains("if (!parameters_compiler::parameters::parse(this, parameters_))"))
                return true;
            else
                return false;
        }

        static bool patch_cpp(const QString& text, const QString& class_name, QString& result)
        {
            QString cpp_text(text);

            QString result_text;
            QString string;
            int index = 0;

            QString on_set_name = QString("%1::OnSetParameters").arg(class_name);
            int on_set_index = cpp_text.indexOf(QRegExp(on_set_name));
            if (on_set_index == -1)
            {
                // Параметров нет, ничего не делаем
                result = text;
                return true;
            }
            on_set_index = cpp_text.indexOf("{", on_set_index);
            if (on_set_index == -1)
                return false;
            on_set_index++;
            if (cpp_text.length() <= on_set_index)
                return false;

            // find last of return core::RC_OK;
            QString return_string = QString("return(\\s+)(core::|core::ReturnCodes::|ReturnCodes::|)RC_OK;");
            int on_set_close_index = cpp_text.indexOf(QRegExp(return_string), on_set_index);
            if (on_set_close_index == -1)
                return false;

            string = cpp_text.left(on_set_close_index);
            result_text.append(string);
            index = on_set_close_index;

            string = QString::fromLocal8Bit("\n\tif (!parameters_compiler::parameters::parse(this, parameters_))\n");
            result_text.append(string);

            string = QString::fromLocal8Bit("\t\tERROR_LOG_RETURN_CODE(core::RC_BAD_PARAM, \"Ошибка при загрузке параметров\");\n\n\t");
            result_text.append(string);

            if (cpp_text.length() > index)
            {
                string = cpp_text.right(cpp_text.length() - index);
                result_text.append(string);
            }

            result = result_text;
            return true;
        }

        static bool get_parameters(const QString& text, const QString& class_name, QStringList& result)
        {
            QString cpp_text(text);

            QString on_set_name = QString("%1::OnSetParameters").arg(class_name);
            int on_set_index = cpp_text.indexOf(QRegExp(on_set_name));
            if (on_set_index == -1)
            {
                // Параметров нет, ничего не делаем
                result = QStringList();
                return true;
            }
            on_set_index = cpp_text.indexOf("{", on_set_index);
            if (on_set_index == -1)
                return false;
            on_set_index++;
            if (cpp_text.length() <= on_set_index)
                return false;

            int on_set_fn_close_index = -1;
            int cnt = 1;
            int pos = on_set_index;
            while (pos < cpp_text.length())
            {
                int t = cpp_text.indexOf("}", pos);
                int t1 = cpp_text.indexOf("{", pos);
                if (t == -1)
                    break;

                if (t > t1)
                {
                    cnt++;
                    pos = t1 == -1 ? t : t1;
                }
                else
                {
                    cnt--;
                    pos = t;
                }

                if (cnt == 0)
                {
                    on_set_fn_close_index = pos;
                    break;
                }

                pos++;
            }
            if (on_set_fn_close_index == -1)
                return false;
            QString on_set_text = cpp_text.mid(on_set_index, on_set_fn_close_index - on_set_index);

            QStringList words;
            QRegularExpression re3(R"wwww(GetParameter(\s+|)\((\s+|)(std::string(\s+|)\(|)("|)(?<param>\w*))wwww");
            re3.setPatternOptions(QRegularExpression::MultilineOption);
            QRegularExpressionMatchIterator i3 = re3.globalMatch(on_set_text);
            while (i3.hasNext())
            {
                QRegularExpressionMatch match = i3.next();
                QString word = match.captured("param");
                words << word;
            }

            QRegularExpression re4(R"wwww(GET_(NUMERIC_|)PARAMETER(_ELRC|)(\s+|)\((\s+|)([\w:,\s]+|)"(?<param>\w*))wwww");
            re4.setPatternOptions(QRegularExpression::MultilineOption);
            QRegularExpressionMatchIterator i4 = re4.globalMatch(on_set_text);
            while (i4.hasNext())
            {
                QRegularExpressionMatch match = i4.next();
                QString word = match.captured("param");
                words << word;
            }

            result = words;
            return true;
        }

        static QString get_parameter_name(const QString& text, const QString& name)
        {
            QString value;
            QString re = QString(R"wwww(const(\s+)std::string(\s+)%1(\s+|)=(\s+|)"(?<value>[\w]+)")wwww").arg(name);
            if (get_single_value(text, re, value))
            {
                return value;
            }
            else
                return QString();
        }

        static QString get_parameter_description(const QString& text, const QString& name)
        {
            QString value;
            QString re = QString(R"wwww(const(\s+)std::string(\s+)%1(\s+|)=(\s+|)"([\w]+)"(\s+|);(\s+|)\/\/!<(\s+|)(?<value>.*))wwww").arg(name);
            if (get_single_value(text, re, value))
            {
                return value;
            }
            else
                return QString();

            return true;
        }
    };
}

#endif // REFACTORING_HELPER_H_
