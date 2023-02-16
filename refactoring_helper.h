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
	};
}

#endif // REFACTORING_HELPER_H_
