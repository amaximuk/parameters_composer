#include <QDebug>
#include <QInputDialog>
#include <QWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QScrollArea>
#include <QCheckBox>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QCloseEvent>
#include <QComboBox>
#include <QProcess>
#include <QDesktopServices>
#include <QSettings>

#include "parameters_compiler_helper.h"
#include "yaml_parser.h"
#include "yaml_writer.h"
#include "json_writer.h"
#include "refactoring_helper.h"
#include "mainwindow.h"

//#ifdef _WIN32 
//const QString eol("\r\n");
//#else
//const QString eol("\n");
//#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    currentFileName_ = "";
    fileInfo_ = {};
    modified_ = false;
    is_json_ = false;

    CreateUi();

    setWindowIcon(QIcon(":/images/parameters.png"));
    UpdateWindowTitle();
}

MainWindow::~MainWindow()
{
}

bool MainWindow::OpenFile(QString fileName)
{
    bool is_json = (QFileInfo(fileName).suffix() == "json");
    return OpenFileInternal(fileName, is_json);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (modified_)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите выйти?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
            event->accept();
        else
            event->ignore();
    }
    else
    {
        event->accept();
    }
}

MainWindow::TabControls& MainWindow::GetTabControls(QString type)
{
    int index = -1;
    for (int i = 0; i < tabs_.size(); i++)
        if (tabs_[i].Name == type)
        {
            index = i;
            break;
        }

    if (index == -1)
    {
        TabControls tc{};
        tc.Name = type;
        tabs_.push_back(std::move(tc));
        index = tabs_.size() - 1;
    }

    return tabs_[index];
}

bool MainWindow::IsTabControlsExists(QString type)
{
    bool found = false;
    for (int i = 0; i < tabs_.size(); i++)
        if (tabs_[i].Name == type)
        {
            found = true;
            break;
        }
    return found;
}

QMap<QString, QObject*>& MainWindow::GetControls(QString type, ControlsGroup group)
{
    MainWindow::TabControls& tc = GetTabControls(type);
    switch (group)
    {
    case MainWindow::ControlsGroup::Info:
        return tc.Info;
    case MainWindow::ControlsGroup::Parameters:
        return tc.Parameters;
    case MainWindow::ControlsGroup::Properties:
        return tc.Properties;
    default:
        return tc.Info; // fake
    }
}

bool MainWindow::RenameTabControlsType(QString oldType, QString newType)
{
    TabControls& tc = GetTabControls(oldType);
    
    tc.Name = newType;

    for (auto& x : tc.Info)
        x->setProperty("type", newType);
    for (auto& x : tc.Parameters)
        x->setProperty("type", newType);
    for (auto& x : tc.Properties)
        x->setProperty("type", newType);
 
    return true;
}

void MainWindow::on_NewFile_action()
{
    if (modified_)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите создать новый файл?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;
    }

    while (tabWidget_->count() > 0)
        tabWidget_->removeTab(0);

    QWidget* widgetTabMain = CreateMainTabWidget();
    tabWidget_->addTab(widgetTabMain, "Main");

    fileInfo_ = {};

    currentFileName_ = "";
    modified_ = false;
    UpdateWindowTitle();
}

void MainWindow::on_Quit_action()
{
    if (modified_)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите выйти?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
            QApplication::quit();
    }
    else
    {
        QApplication::quit();
    }
}

void MainWindow::on_Apply_action()
{
    QFileDialog dialog(this);
    dialog.setNameFilter("CMake Files (CMakeLists.txt)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() == 0)
        return;

    ApplyInternal(fileNames[0]);
}

void MainWindow::on_OpenFile_action()
{
    if (modified_)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите открыть файл?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;
    }

    QFileDialog dialog(this);
    dialog.setNameFilters({ "Parameters Compiler YAML Files (*.yml *.yaml)", "Parameters Compiler JSON Files (*.json)" });
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() == 0)
        return;

    bool is_json = (dialog.selectedNameFilter() == "Parameters Compiler JSON Files (*.json)");

    OpenFileInternal(fileNames[0], is_json);
}

bool MainWindow::OpenFileInternal(QString fileName, bool is_json)
{
    qDebug() << "Opening " << fileName;

    if (!QFile::exists(fileName))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Файл %1 не найден").arg(fileName));
        RemoveRecent(fileName);
        return false;
    }

    parameters_compiler::file_info fi{};
    if (!yaml::parser::parse(fileName.toStdString(), fi))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Ошибка разбора файла %1").arg(fileName));
        return false;
    }

    while (tabWidget_->count() > 1)
        tabWidget_->removeTab(1);

    TabControls& tc = GetTabControls("Main");
    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
    listWidget->clear();

    fileInfo_ = fi;

    UpdateMain();
    for (const auto& type : parameters_compiler::helper::get_user_type_names(fileInfo_))
    {
        QWidget* widgetTabType = CreateTypeTabWidget(QString::fromStdString(type));
        tabWidget_->addTab(widgetTabType, QString::fromStdString(type));
        UpdateType(QString::fromStdString(type));
    }

    FillPropertyTypeNames();

    currentFileName_ = fileName;
    modified_ = false;
    UpdateWindowTitle();
    AddRecent(fileName);

    return true;
}

bool MainWindow::ApplyInternal(QString fileName)
{
    qDebug() << "Apply to " << fileName;

    if (!QFile::exists(fileName))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Файл %1 не найден").arg(fileName));
        return false;
    }

    QString cmake_text;
    if (!refactoring::helper::read_file(fileName, "UTF-8", cmake_text))
        return false;

    QString target_name;
    if (!refactoring::helper::get_single_value(cmake_text, R"wwww((SET|set)(\s|)\((\s|)TARGET_NAME\s(?<value>\w*))wwww", target_name))
        return false;

    QString cmake_patched;
    if (!refactoring::helper::patch_cmake(cmake_text, cmake_patched))
        return false;


    QString params_path = QFileInfo(fileName).absoluteDir().filePath("params");
    if (!QFile::exists(params_path))
        if (!QDir().mkpath(params_path))
            return false;

    QString target_file_name = QString("%1%2").arg(target_name, ".yml");
    QString params_file_name = QDir(params_path).filePath(target_file_name);

    QFile fw(params_file_name);
    if (!fw.open(QFile::ReadWrite | QFile::Text))
        return false;
    QTextStream out(&fw);
    out.setCodec("UTF-8");
    out << result_text;
    fw.close();

    QString lib_class_file_name;
    if (QFile::exists(QFileInfo(fileName).absoluteDir().filePath("lib_class_name.h")))
        lib_class_file_name = QFileInfo(fileName).absoluteDir().filePath("lib_class_name.h");
    else if (QFile::exists(QFileInfo(fileName).absoluteDir().filePath("src/lib_class_name.h")))
        lib_class_file_name = QFileInfo(fileName).absoluteDir().filePath("src/lib_class_name.h");
    else
        return false;

    QFile frl(lib_class_file_name);
    if (!frl.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream inl(&frl);
    inl.setCodec("windows-1251");
    QString lib_class_text = inl.readAll();
    //lib_class_text = "#define LIBRARY_CLASS_NAME Rpu6CanReceiver";
    frl.close();

    QString class_name;
    QRegularExpression re(R"wwww(^#define LIBRARY_CLASS_NAME (?<name>\w*))wwww");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = re.match(lib_class_text);
    if (match.hasMatch())
        class_name = match.captured("name");

    QString include_file;
    QRegularExpression re2(R"wwww(^#include "(?<include>.*)")wwww");
    re2.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatch match2 = re2.match(lib_class_text);
    if (match2.hasMatch())
        include_file = match2.captured("include");




    QString include_file_name = QFileInfo(fileName).absoluteDir().filePath(include_file);
    QFile frcl(include_file_name);
    if (!frcl.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream incl(&frcl);
    incl.setCodec("windows-1251");
    QString include_text = incl.readAll();
    //lib_class_text = "#define LIBRARY_CLASS_NAME Rpu6CanReceiver";
    frcl.close();



    int class_include_index = include_text.indexOf("#include \"base_library/base_library.h\"");
    if (class_include_index == -1)
        return false;
    int class_include_close_index = include_text.indexOf("\n", class_include_index);
    if (class_include_close_index == -1)
        return false;


    QString class_class_name = QString("class %1").arg(class_name);
    int class_class_index = include_text.indexOf(class_class_name);
    if (class_class_index == -1)
        return false;
    class_class_index = include_text.indexOf("{", class_class_index);
    if (class_class_index == -1)
        return false;
    class_class_index++;
    if (include_text.length() <= class_class_index)
        return false;

    QString result_header;

    string = include_text.left(class_include_close_index);
    result_header.append(string);
    index = class_include_close_index;

    string = QString::fromLocal8Bit("\n#include \"%1.yml.h\"").arg(target_name);
    result_header.append(string);

    string = include_text.mid(index, class_class_index - index + 1);
    result_header.append(string);
    index += class_class_index - index + 1;

    string = QString::fromLocal8Bit("\nprivate:\n\tparameters_compiler::parameters parameters_;\n");
    result_header.append(string);

    string = include_text.right(include_text.length() - class_class_index);
    result_header.append(string);







    QFileInfo include_file_info(include_file_name);
    QString cpp_file_name = include_file_info.path() + "/" + include_file_info.completeBaseName() + ".cpp";


    QFile frclc(cpp_file_name);
    if (!frclc.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream inclc(&frclc);
    inclc.setCodec("windows-1251");
    QString cpp_text = inclc.readAll();
    //lib_class_text = "#define LIBRARY_CLASS_NAME Rpu6CanReceiver";
    frclc.close();



    QString on_set_name = QString("%1::OnSetParameters()").arg(class_name);
    int on_set_index = cpp_text.indexOf(on_set_name);
    if (on_set_index == -1)
        return false;
    on_set_index = cpp_text.indexOf("{", on_set_index);
    if (on_set_index == -1)
        return false;
    on_set_index++;
    if (cpp_text.length() <= on_set_index)
        return false;

    // find last of return core::RC_OK;
    int on_set_close_index = cpp_text.indexOf("return core::RC_OK;", on_set_index);
    if (on_set_close_index == -1)
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

    QRegularExpression re3(R"wwww(GetParameter(\s|)\((\s|)(?<param>\w*))wwww");
    re3.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator i3 = re3.globalMatch(on_set_text);
    while (i3.hasNext())
    {
        QRegularExpressionMatch match = i3.next();
        QString word = match.captured("param");
        words << word;
    }

    QRegularExpression re4(R"wwww(GET_(NUMERIC_|)PARAMETER(_ELRC|)(\s|)\((\s|)([\w:,\s]+|)"(?<param>\w*))wwww");
    re4.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator i4 = re4.globalMatch(on_set_text);
    while (i4.hasNext())
    {
        QRegularExpressionMatch match = i4.next();
        QString word = match.captured("param");
        words << word;
    }











    QString result_cpp;

    string = cpp_text.left(on_set_close_index);
    result_cpp.append(string);
    index = on_set_close_index;

    string = QString::fromLocal8Bit("\n\tif (!parameters_compiler::parameters::parse(this, parameters_);)\n").arg(target_name);
    result_cpp.append(string);
    string = QString::fromLocal8Bit("\t\tERROR_LOG_RETURN_CODE(core::RC_BAD_PARAM, \"Ошибка при загрузке параметров\");\n\n\t").arg(target_name);
    result_cpp.append(string);

    if (cpp_text.length() > index)
    {
        string = cpp_text.right(cpp_text.length() - index);
        result_cpp.append(string);
    }

    return true;
}

void MainWindow::UpdateRecent()
{
    recentMenu_->clear();

    QString iniFileName = QDir(QCoreApplication::applicationDirPath()).filePath("settings.ini");
    QSettings app_settings(iniFileName, QSettings::IniFormat);
    int recent_count = app_settings.value("recent/count", "0").toInt();
    if (recent_count == 0)
    {
        QAction* recentAct = new QAction(QString::fromLocal8Bit("<список пуст>"), this);
        recentAct->setStatusTip(QString::fromLocal8Bit("Список пуст"));
        recentMenu_->addAction(recentAct);
    }
    else
    {
        for (int i = 0; i < recent_count; i++)
        {
            QString name = QString("recent/filename_%1").arg(i);
            QString path = app_settings.value(name, "").toString();
            QAction* recentAct = new QAction(path, this);
            recentAct->setStatusTip(QString::fromLocal8Bit("Открыть файл %1").arg(path));
            connect(recentAct, &QAction::triggered, this, &MainWindow::on_Recent_action);
            recentMenu_->addAction(recentAct);
        }
    }
}

void MainWindow::AddRecent(QString fileName)
{
    QString iniFileName = QDir(QCoreApplication::applicationDirPath()).filePath("settings.ini");
    QSettings app_settings(iniFileName, QSettings::IniFormat);
    int recent_count = app_settings.value("recent/count", "0").toInt();
    QList<QString> list;
    for (int i = 0; i < recent_count; i++)
    {
        QString name = QString("recent/filename_%1").arg(i);
        QString path = app_settings.value(name, "").toString();
        path.replace("\\", "/");
        list.push_back(path);
    }

    app_settings.beginGroup("recent");
    app_settings.remove(""); //removes the group, and all it keys
    app_settings.endGroup();

    fileName.replace("\\", "/");
    if (list.contains(fileName))
        list.removeAll(fileName);
    list.push_front(fileName);
    while (list.size() > 10)
        list.pop_back();

    app_settings.setValue("recent/count", list.size());
    for (int i = 0; i < list.size(); i++)
    {
        QString name = QString("recent/filename_%1").arg(i);
        app_settings.setValue(name, list[i]);
    }
    app_settings.sync();

    UpdateRecent();
}

void MainWindow::RemoveRecent(QString fileName)
{
    QString iniFileName = QDir(QCoreApplication::applicationDirPath()).filePath("settings.ini");
    QSettings app_settings(iniFileName, QSettings::IniFormat);
    int recent_count = app_settings.value("recent/count", "0").toInt();
    QList<QString> list;
    for (int i = 0; i < recent_count; i++)
    {
        QString name = QString("recent/filename_%1").arg(i);
        QString path = app_settings.value(name, "").toString();
        path.replace("\\", "/");
        list.push_back(path);
    }

    app_settings.beginGroup("recent");
    app_settings.remove(""); //removes the group, and all it keys
    app_settings.endGroup();

    fileName.replace("\\", "/");
    if (list.contains(fileName))
        list.removeAll(fileName);
    while (list.size() > 10)
        list.pop_back();

    app_settings.setValue("recent/count", list.size());
    for (int i = 0; i < list.size(); i++)
    {
        QString name = QString("recent/filename_%1").arg(i);
        app_settings.setValue(name, list[i]);
    }
    app_settings.sync();

    UpdateRecent();
}

void MainWindow::on_SaveFile_action()
{
    if (!modified_)
        return;

    if (currentFileName_ == "")
    {
        SaveAs();
    }
    else
    {
        qDebug() << currentFileName_;

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите сохранить файл?\nФайл будет перезаписан!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        if (!ReadCurrentFileInfo())
            return;

        std::string message;
        if (!parameters_compiler::helper::validate(fileInfo_, message))
        {
            QMessageBox::critical(this, "Validate error", QString::fromLocal8Bit(message.c_str()));
            return;
        }

        bool have_type_loop = false;
        if (!parameters_compiler::helper::rearrange_types(fileInfo_, have_type_loop))
        {
            QMessageBox::critical(this, "Rearrange error", QString::fromLocal8Bit("Ошибка переупорядочивания пользовательских типов перед сохранением"));
            return;
        }
        else if (have_type_loop)
        {
            QMessageBox::warning(this, "Rearrange", QString::fromLocal8Bit("Обнаружена циклицеская зависимость в типах.\nФайл будет сохранен, но эта логическая ошибка требует исправления!"));
        }

        if (is_json_)
        {
            if (!json::writer::write(currentFileName_.toStdString(), fileInfo_))
                return;
        }
        else
        {
            if (!yaml::writer::write(currentFileName_.toStdString(), fileInfo_))
                return;
        }

        modified_ = false;
        UpdateWindowTitle();
    }
}

void MainWindow::on_SaveAsFile_action()
{
    SaveAs();
}

void MainWindow::on_AddType_action()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add type", QString::fromLocal8Bit("Имя типа:"), QLineEdit::Normal, "", &ok);
    if (!ok || text.isEmpty())
        return;

    for (const auto& ti : fileInfo_.types)
    {
        if (QString::fromStdString(ti.name) == text)
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Тип с именем %1 уже существует").arg(text));
            return;
        }
    }

    QWidget* widgetTabType = CreateTypeTabWidget(text);
    tabWidget_->addTab(widgetTabType, text);
    tabWidget_->setCurrentWidget(widgetTabType);

    TabControls& tc = GetTabControls(text);
    qobject_cast<QLineEdit*>(tc.Info["NAME"])->setText(text);

    parameters_compiler::type_info ti{};
    ti.name = text.toStdString();
    fileInfo_.types.push_back(ti);

    //FillTypeTypeNames();
    FillPropertyTypeNames();

    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::on_RemoveType_action()
{
    QString name = tabWidget_->tabText(tabWidget_->indexOf(tabWidget_->currentWidget()));

    if (name == "Main")
    {
        QMessageBox::warning(this, "parameters_composer", QString::fromLocal8Bit("Главная страница не может быть удалена"));
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        QString::fromLocal8Bit("Вы действительно хотите удалить тип %1?").arg(name), QMessageBox::No | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
        return;

    QList<QString> usedInTypes;
    QString arrayName = QString("array<%1>").arg(name);

    // Update fileInfo_
    for (auto& p : fileInfo_.parameters)
    {
        if (QString::fromStdString(p.type) == name || QString::fromStdString(p.type) == arrayName)
        {
            usedInTypes.push_back("Main");
            break;
        }
    }
    for (auto& t : fileInfo_.types)
    {
        for (auto& p : t.parameters)
        {
            if (QString::fromStdString(p.type) == name || QString::fromStdString(p.type) == arrayName)
            {
                usedInTypes.push_back(QString::fromStdString(t.name));
                break;
            }
        }
    }

    if (usedInTypes.size() > 0)
    {
        QString message = QString::fromLocal8Bit("Тип %1 используется в других типах:\n").arg(name);
        for (const auto& s : usedInTypes)
            message += s + "\n";
        QMessageBox::critical(this, "parameters_composer", message);
        return;
    }

    tabWidget_->removeTab(tabWidget_->indexOf(tabWidget_->currentWidget()));
    auto it = std::find_if(fileInfo_.types.cbegin(), fileInfo_.types.cend(), [name](auto& t) { if (t.name == name.toStdString()) return true; else return false; });
    fileInfo_.types.erase(it);

    FillPropertyTypeNames();

    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::on_Compile_action()
{
    if (Compile())
        QMessageBox::information(this, "parameters_composer", QString::fromLocal8Bit("Файл успешно скомпилирован"));
}

bool MainWindow::Compile()
{
    QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
    if (!SaveAsInternal(QDir(workingDir).filePath("temp.yml"), false, true))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Ошибка при сохранении файла YAML"));
        return false;
    }

    if (!SaveAsInternal(QDir(workingDir).filePath("temp.json"), true, true))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Ошибка при сохранении файла JSON"));
        return false;
    }

    QProcess processYaml;
    processYaml.setProgram(QDir(workingDir).filePath("parameters_compiler.exe"));
    processYaml.setArguments({ "temp.yml", "-o", "-t" });
    processYaml.setWorkingDirectory(workingDir);
    processYaml.start();
    if (!processYaml.waitForFinished(1000) || processYaml.exitCode() != 0)
    {
        QString output(processYaml.readAllStandardOutput());
        if (output == "") output = QString::fromLocal8Bit("Неизвестная ошибка");
        QMessageBox::critical(this, "Compile YAML", output);
        return false;
    }

    QProcess processJson;
    processJson.setProgram(QDir(workingDir).filePath("parameters_compiler.exe"));
    processJson.setArguments({ "temp.json", "-o", "-t" });
    processJson.setWorkingDirectory(workingDir);
    processJson.start();
    if (!processJson.waitForFinished(1000) || processJson.exitCode() != 0)
    {
        QString output(processJson.readAllStandardOutput());
        if (output == "") output = QString::fromLocal8Bit("Неизвестная ошибка");
        QMessageBox::critical(this, "Compile JSON", output);
        return false;
    }

    QFile fileYaml(QDir(workingDir).filePath("temp.yml.h"));
    if (!fileYaml.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл кода, скомпилированный из YAML не найден"));
        return false;
    }

    QFile fileJson(QDir(workingDir).filePath("temp.json.h"));
    if (!fileJson.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл JSON не найден"));
        return false;
    }

    if (fileYaml.size() != fileJson.size())
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файлы YAML и JSON отличаются по размеру"));
        return false;
    }

    QByteArray bytesYaml = fileYaml.readAll();
    QByteArray bytesJson = fileJson.readAll();
    if (!std::equal(bytesYaml.cbegin(), bytesYaml.cend(), bytesJson.cbegin()))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файлы YAML и JSON отличаются по содержимому"));
        return false;
    }

    return true;
}

void MainWindow::on_ViewYaml_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.yml")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл temp.yml не найден.\nЗапустите компиляцию!"));
            return;
        }
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("temp.yml"))));
    }
}

void MainWindow::on_ViewJson_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.json")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл temp.json не найден.\nЗапустите компиляцию!"));
            return;
        }
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("temp.json"))));
    }
}

void MainWindow::on_ViewCode_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.yml.h")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл temp.yml.h не найден.\nЗапустите компиляцию!"));
            return;
        }
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("temp.yml.h"))));
    }
}

void MainWindow::on_ViewHtml_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.yml.html")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл temp.yml.html не найден.\nЗапустите компиляцию!"));
            return;
        }
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("temp.yml.html"))));
    }
}

void MainWindow::on_OpenFolder_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(workingDir)));
    }
}

void MainWindow::on_Help_action()
{
    QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
    if (!QFileInfo::exists(QDir(workingDir).filePath("parameters_compiler.pdf")))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("Файл parameters_compiler.pdf не найден"));
        return;
    }
    QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("parameters_compiler.pdf"))));
}

void MainWindow::on_Recent_action()
{
    QAction* act = qobject_cast<QAction*>(sender());
    OpenFile(act->text());
}

void MainWindow::CreateMenu()
{
    QAction* newAct = new QAction(QString::fromLocal8Bit("Создать"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(QString::fromLocal8Bit("Создать новый файл"));
    connect(newAct, &QAction::triggered, this, &MainWindow::on_NewFile_action);

    QAction* openAct = new QAction(QString::fromLocal8Bit("Открыть"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(QString::fromLocal8Bit("Открыть файл"));
    connect(openAct, &QAction::triggered, this, &MainWindow::on_OpenFile_action);

    QAction* saveAct = new QAction(QString::fromLocal8Bit("Сохранить"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(QString::fromLocal8Bit("Сохранить файл"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::on_SaveFile_action);

    QAction* saveAsAct = new QAction(QString::fromLocal8Bit("Сохранить как..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(QString::fromLocal8Bit("Сохранить файл как..."));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::on_SaveAsFile_action);

    QAction* applyAct = new QAction(QString::fromLocal8Bit("Внедрить"), this);
    applyAct->setShortcuts(QKeySequence::SaveAs);
    applyAct->setStatusTip(QString::fromLocal8Bit("Внедрить в проект"));
    connect(applyAct, &QAction::triggered, this, &MainWindow::on_Apply_action);

    QAction* quitAct = new QAction(QString::fromLocal8Bit("Выйти"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(QString::fromLocal8Bit("Выйти из приложения"));
    connect(quitAct, &QAction::triggered, this, &MainWindow::on_Quit_action);

    QMenu* fileMenu = menuBar()->addMenu(QString::fromLocal8Bit("Файл"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(applyAct);
    fileMenu->addSeparator();
    recentMenu_ = fileMenu->addMenu(QString::fromLocal8Bit("Недавние файлы"));
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    QAction* addTypeAct = new QAction(QString::fromLocal8Bit("Добавить"), this);
    addTypeAct->setStatusTip(QString::fromLocal8Bit("Добавить тип"));
    connect(addTypeAct, &QAction::triggered, this, &MainWindow::on_AddType_action);

    QAction* removeTypeAct = new QAction(QString::fromLocal8Bit("Удалить"), this);
    removeTypeAct->setStatusTip(QString::fromLocal8Bit("Удалить тип"));
    connect(removeTypeAct, &QAction::triggered, this, &MainWindow::on_RemoveType_action);

    QMenu* typeMenu = menuBar()->addMenu(QString::fromLocal8Bit("Типы"));
    typeMenu->addAction(addTypeAct);
    typeMenu->addAction(removeTypeAct);

    QAction* compileAct = new QAction(QString::fromLocal8Bit("Тест компиляции"), this);
    compileAct->setStatusTip(QString::fromLocal8Bit("Компилировать временный файл"));
    connect(compileAct, &QAction::triggered, this, &MainWindow::on_Compile_action);

    QAction* viewYamlAct = new QAction(QString::fromLocal8Bit("Просмотр YAML"), this);
    viewYamlAct->setStatusTip(QString::fromLocal8Bit("Предварительный просмотр YAML файла"));
    connect(viewYamlAct, &QAction::triggered, this, &MainWindow::on_ViewYaml_action);

    QAction* viewJsonAct = new QAction(QString::fromLocal8Bit("Просмотр JSON"), this);
    viewJsonAct->setStatusTip(QString::fromLocal8Bit("Предварительный просмотр JSON файла"));
    connect(viewJsonAct, &QAction::triggered, this, &MainWindow::on_ViewJson_action);

    QAction* viewCodeAct = new QAction(QString::fromLocal8Bit("Просмотр кода"), this);
    viewCodeAct->setStatusTip(QString::fromLocal8Bit("Предварительный просмотр .h файла"));
    connect(viewCodeAct, &QAction::triggered, this, &MainWindow::on_ViewCode_action);

    QAction* viewHtmlAct = new QAction(QString::fromLocal8Bit("Просмотр HTML"), this);
    viewHtmlAct->setStatusTip(QString::fromLocal8Bit("Предварительный просмотр HTML файла"));
    connect(viewHtmlAct, &QAction::triggered, this, &MainWindow::on_ViewHtml_action);

    QAction* openFolderAct = new QAction(QString::fromLocal8Bit("Показать в папке"), this);
    openFolderAct->setStatusTip(QString::fromLocal8Bit("Показать файлы в папке"));
    connect(openFolderAct, &QAction::triggered, this, &MainWindow::on_OpenFolder_action);

    QMenu* compileMenu = menuBar()->addMenu(QString::fromLocal8Bit("Компилятор"));
    compileMenu->addAction(compileAct);
    compileMenu->addSeparator();
    compileMenu->addAction(viewYamlAct);
    compileMenu->addAction(viewJsonAct);
    compileMenu->addAction(viewCodeAct);
    compileMenu->addAction(viewHtmlAct);
    compileMenu->addSeparator();
    compileMenu->addAction(openFolderAct);

    QAction* helpAct = new QAction(QString::fromLocal8Bit("Открыть описание"), this);
    helpAct->setStatusTip(QString::fromLocal8Bit("Открыть описание формата конфигурационных файлов"));
    connect(helpAct, &QAction::triggered, this, &MainWindow::on_Help_action);

    QMenu* helpMenu = menuBar()->addMenu(QString::fromLocal8Bit("Помощь"));
    helpMenu->addAction(helpAct);

    UpdateRecent();
}

void MainWindow::CreateUi()
{
    resize(1000, 600);

    CreateMenu();

    //QTabWidget* tabWidget = new QTabWidget;
    //QWidget* widgetTabProperties = CreateMainTabWidget();
    //tabWidget->addTab(widgetTabProperties, "Main");
    //setCentralWidget(tabWidget);

    focusFilter_ = new FocusFilter;
    connect(focusFilter_, &FocusFilter::onFocusChanged, this, &MainWindow::on_FocusChanged);

    QWidget* mainWidget = CreateMainWidget();
    setCentralWidget(mainWidget);
}

QWidget* MainWindow::CreateMainWidget()
{
    QWidget* widgetMainProperties = new QWidget;

    QWidget* hintWidget = new QWidget;
    plainTextEditHint_ = new QPlainTextEdit;
    plainTextEditHint_->setFixedHeight(100);
    plainTextEditHint_->setReadOnly(true);
    QVBoxLayout* vBoxLayoutHint = new QVBoxLayout;
    vBoxLayoutHint->setMargin(0);
    vBoxLayoutHint->addWidget(plainTextEditHint_);
    hintWidget->setLayout(vBoxLayoutHint);

    tabWidget_ = new QTabWidget;
    QWidget* widgetTabProperties = CreateMainTabWidget();
    tabWidget_->addTab(widgetTabProperties, "Main");

    QSplitter* tabVSplitter = new QSplitter(Qt::Vertical);
    tabVSplitter->addWidget(tabWidget_);
    tabVSplitter->addWidget(hintWidget);
    tabVSplitter->setStretchFactor(0, 1);
    tabVSplitter->setStretchFactor(1, 0);

    QVBoxLayout* vBoxLayoutSplitter = new QVBoxLayout;
    vBoxLayoutSplitter->addWidget(tabVSplitter);
    widgetMainProperties->setLayout(vBoxLayoutSplitter);

    return widgetMainProperties;
}

QWidget* MainWindow::CreateMainTabWidget()
{
    QWidget* widgetTabProperties = new QWidget;

    QWidget* widgetSplitterInfo = CreateMainTabInfoWidget();
    QWidget* widgetSplitterPropertyList = CreatePropertyListWidget("Main");
    QWidget* widgetSplitterProperties = CreatePropertiesWidget("Main");

    AddGroupWidget(widgetSplitterInfo, "INFO_GROUP", "Main", ControlsGroup::Info);
    AddGroupWidget(widgetSplitterPropertyList, "PARAMETERS_GROUP", "Main", ControlsGroup::Parameters);
    AddGroupWidget(widgetSplitterProperties, "PROPERTIES_GROUP", "Main", ControlsGroup::Properties);

    QSplitter* tabHSplitter = new QSplitter(Qt::Horizontal);
    tabHSplitter->addWidget(widgetSplitterInfo);
    tabHSplitter->addWidget(widgetSplitterPropertyList);
    tabHSplitter->addWidget(widgetSplitterProperties);
    tabHSplitter->setStretchFactor(0, 1);
    tabHSplitter->setStretchFactor(1, 0);
    tabHSplitter->setStretchFactor(2, 1);
    widgetSplitterProperties->setEnabled(false);

    QVBoxLayout* vBoxLayoutSplitter = new QVBoxLayout;
    vBoxLayoutSplitter->addWidget(tabHSplitter);
    widgetTabProperties->setLayout(vBoxLayoutSplitter);

    return widgetTabProperties;
}

QWidget* MainWindow::CreateMainTabInfoWidget()
{
    QGridLayout* gridLayoutInfo = new QGridLayout;
 
    QString type("Main");
    TabControls& tc = GetTabControls(type);

    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "INFO", "font-weight: bold; font-size: 14px", index++);
    AddLineEditProperty(gridLayoutInfo, "ID", index++, type, ControlsGroup::Info, true);
    AddLineEditProperty(gridLayoutInfo, "DISPLAY_NAME", index++, type, ControlsGroup::Info, false);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "CATEGORY", index++, type, ControlsGroup::Info, false);
    AddLineEditProperty(gridLayoutInfo, "PICTOGRAM", index++, type, ControlsGroup::Info, false);
    AddLineEditProperty(gridLayoutInfo, "HINT", index++, type, ControlsGroup::Info, false);
    AddLineEditProperty(gridLayoutInfo, "AUTHOR", index++, type, ControlsGroup::Info, false);
    AddLineEditProperty(gridLayoutInfo, "WIKI", index++, type, ControlsGroup::Info, false);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

QWidget* MainWindow::CreatePropertyListWidget(QString type)
{
    QLabel* labelPropertyListHeader = new QLabel;
    labelPropertyListHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelPropertyListHeader->setText("PARAMETERS");

    QWidget* widgetPropertyListButtons = CreateListControlWidget(32, type, ControlsGroup::Parameters, "PARAMETERS", QString::fromLocal8Bit("параметр"));

    QListWidget* listWidget = new QListWidget;
    listWidget->setProperty("name", "PARAMETERS");
    listWidget->setProperty("group", static_cast<int>(ControlsGroup::Parameters));
    listWidget->setProperty("type", type);
    listWidget->installEventFilter(focusFilter_);

    connect(listWidget, &QListWidget::currentItemChanged, this, &MainWindow::on_CurrentItemChanged);

    QVBoxLayout* vBoxLayoutPropertyList = new QVBoxLayout;
    vBoxLayoutPropertyList->addWidget(labelPropertyListHeader, 0, Qt::AlignCenter);
    vBoxLayoutPropertyList->addWidget(widgetPropertyListButtons);
    vBoxLayoutPropertyList->addWidget(listWidget, 1);
    vBoxLayoutPropertyList->addStretch();

    QWidget* widgetSplitterPropertyList = new QWidget;
    widgetSplitterPropertyList->setLayout(vBoxLayoutPropertyList);

    auto& tc = GetControls(type, ControlsGroup::Parameters);
    tc["PARAMETERS"] = listWidget;

    return widgetSplitterPropertyList;
}

void MainWindow::AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, MainWindow::ControlsGroup group, bool is_bold)
{
    QLabel* label = new QLabel;
    if (is_bold)
        label->setStyleSheet("font-weight: bold");
    label->setText(name);

    gridLayout->addWidget(label, index, 0);
    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setProperty("name", name);
    lineEdit->setProperty("group", static_cast<int>(group));
    lineEdit->setProperty("type", type);
    lineEdit->installEventFilter(focusFilter_);

    connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::on_EditingFinished);
    gridLayout->addWidget(lineEdit, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = lineEdit;
}

void MainWindow::AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QPlainTextEdit* plainTextEdit = new QPlainTextEdit;
    plainTextEdit->setProperty("name", name);
    plainTextEdit->setProperty("group", static_cast<int>(group));
    plainTextEdit->setProperty("type", type);
    plainTextEdit->installEventFilter(focusFilter_);

    connect(plainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::on_TextChanged);
    gridLayout->addWidget(plainTextEdit, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = plainTextEdit;
}

void MainWindow::AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QCheckBox* checkBox = new QCheckBox;
    checkBox->setProperty("name", name);
    checkBox->setProperty("group", static_cast<int>(group));
    checkBox->setProperty("type", type);
    checkBox->installEventFilter(focusFilter_);

    connect(checkBox, &QCheckBox::stateChanged, this, &MainWindow::on_StateChanged);
    gridLayout->addWidget(checkBox, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = checkBox;
}

void MainWindow::AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index)
{
    QLabel* label = new QLabel;
    label->setStyleSheet(style);
    label->setText(text);
    gridLayout->addWidget(label, index, 0, 1, 2, Qt::AlignCenter);
}

void MainWindow::AddListProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    QWidget* widgetPropertiesRestrictionsSetButtons = CreateListControlWidget(24, type, group, name, QString::fromLocal8Bit("значение %1").arg(name));
    QListWidget* listWidget = new QListWidget;
    listWidget->setProperty("name", name);
    listWidget->setProperty("group", static_cast<int>(group));
    listWidget->setProperty("type", type);
    listWidget->installEventFilter(focusFilter_);

    QVBoxLayout* vBoxLayoutPropertiesRestrictionsSet = new QVBoxLayout;
    vBoxLayoutPropertiesRestrictionsSet->addWidget(widgetPropertiesRestrictionsSetButtons);
    vBoxLayoutPropertiesRestrictionsSet->addWidget(listWidget, 1);
    vBoxLayoutPropertiesRestrictionsSet->addStretch();
    vBoxLayoutPropertiesRestrictionsSet->setMargin(0);
    QWidget* widgetPropertiesRestrictionsSet = new QWidget;
    widgetPropertiesRestrictionsSet->setLayout(vBoxLayoutPropertiesRestrictionsSet);
    gridLayout->addWidget(new QLabel(name), index, 0);
    gridLayout->addWidget(widgetPropertiesRestrictionsSet, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = listWidget;
}

void MainWindow::AddComboBoxPropertyType(QGridLayout* gridLayout, QString name, int index, QString type, MainWindow::ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);

    QComboBox* comboBox = new QComboBox;
    comboBox->setEditable(false);
    comboBox->setProperty("name", name);
    comboBox->setProperty("group", static_cast<int>(group));
    comboBox->setProperty("type", type);
    comboBox->installEventFilter(focusFilter_);

    for (const auto& s : parameters_compiler::helper::get_property_type_names(fileInfo_))
        comboBox->addItem(QString::fromStdString(s));

    gridLayout->addWidget(comboBox, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = comboBox;
}

void MainWindow::AddComboBoxTypeType(QGridLayout* gridLayout, QString name, int index, QString type, MainWindow::ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);

    QComboBox* comboBox = new QComboBox;
    comboBox->setEditable(false);
    comboBox->setProperty("name", name);
    comboBox->setProperty("group", static_cast<int>(group));
    comboBox->setProperty("type", type);
    comboBox->installEventFilter(focusFilter_);

    for (const auto& s : parameters_compiler::helper::get_type_type_names())
        comboBox->addItem(QString::fromStdString(s));

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_CurrentIndexChanged);
    gridLayout->addWidget(comboBox, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = comboBox;
}

void MainWindow::AddGroupWidget(QWidget* groupWidget, QString name, QString type, ControlsGroup group)
{
    auto& tc = GetControls(type, group);
    tc[name] = groupWidget;
}

bool MainWindow::ReadCurrentParameter(QString type, parameters_compiler::parameter_info& pi)
{
    TabControls& tc = GetTabControls(type);

    pi.name = qobject_cast<QLineEdit*>(tc.Properties["NAME"])->text().toStdString();
    pi.type = qobject_cast<QComboBox*>(tc.Properties["TYPE"])->currentText().toStdString();
    pi.display_name = qobject_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->text().toStdString();
    pi.description = qobject_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->toPlainText().toStdString();
    pi.required = qobject_cast<QCheckBox*>(tc.Properties["REQUIRED"])->isChecked();
    pi.default_ = qobject_cast<QLineEdit*>(tc.Properties["DEFAULT"])->text().toStdString();
    pi.hint = qobject_cast<QLineEdit*>(tc.Properties["HINT"])->text().toStdString();

    pi.restrictions.min = qobject_cast<QLineEdit*>(tc.Properties["MIN"])->text().toStdString();
    pi.restrictions.max = qobject_cast<QLineEdit*>(tc.Properties["MAX"])->text().toStdString();
    pi.restrictions.set_.clear();
    QListWidget* listWidgetSet = qobject_cast<QListWidget*>(tc.Properties["SET"]);
    for (int i = 0; i < listWidgetSet->count(); ++i)
        pi.restrictions.set_.push_back(listWidgetSet->item(i)->text().toStdString());

    pi.restrictions.min_count = qobject_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->text().toStdString();
    pi.restrictions.max_count = qobject_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->text().toStdString();
    pi.restrictions.set_count.clear();
    QListWidget* listWidgetSetCount = qobject_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    for (int i = 0; i < listWidgetSetCount->count(); ++i)
        pi.restrictions.set_count.push_back(listWidgetSetCount->item(i)->text().toStdString());

    pi.restrictions.category = qobject_cast<QLineEdit*>(tc.Properties["CATEGORY"])->text().toStdString();

    pi.restrictions.ids.clear();
    QListWidget* listWidgetIds = qobject_cast<QListWidget*>(tc.Properties["IDS"]);
    for (int i = 0; i < listWidgetIds->count(); ++i)
        pi.restrictions.ids.push_back(listWidgetIds->item(i)->text().toStdString());

    pi.restrictions.max_length = qobject_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->text().toStdString();

    return true;
}

bool MainWindow::HaveCurrentParameter(QString type)
{
    TabControls& tc = GetTabControls(type);
    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
    return (listWidget->selectedItems().size() > 0);
}

bool MainWindow::ReadCurrentMainInfo(parameters_compiler::info_info& mi)
{
    TabControls& tc = GetTabControls("Main");

    mi.id = qobject_cast<QLineEdit*>(tc.Info["ID"])->text().toStdString();
    mi.display_name = qobject_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->text().toStdString();
    mi.description = qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->toPlainText().toStdString();
    mi.category = qobject_cast<QLineEdit*>(tc.Info["CATEGORY"])->text().toStdString();
    mi.pictogram = qobject_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->text().toStdString();
    mi.hint = qobject_cast<QLineEdit*>(tc.Info["HINT"])->text().toStdString();
    mi.author = qobject_cast<QLineEdit*>(tc.Info["AUTHOR"])->text().toStdString();
    mi.wiki = qobject_cast<QLineEdit*>(tc.Info["WIKI"])->text().toStdString();

    return true;
}

bool MainWindow::ReadCurrentTypeInfo(QString type, parameters_compiler::type_info& ti)
{
    TabControls& tc = GetTabControls(type);

    ti.name = qobject_cast<QLineEdit*>(tc.Info["NAME"])->text().toStdString();
    ti.type = qobject_cast<QComboBox*>(tc.Info["TYPE"])->currentText().toStdString();
    ti.description = qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->toPlainText().toStdString();

    ti.values.clear();
    QListWidget* listWidgetValues = qobject_cast<QListWidget*>(tc.Info["VALUES"]);
    for (int i = 0; i < listWidgetValues->count(); ++i)
    {
        QString v = listWidgetValues->item(i)->text();
        QStringList sl = v.split(" -> ");
        ti.values.push_back({ sl[0].toStdString(), sl[1].toStdString() });
    }

    ti.includes.clear();
    QListWidget* listWidgetIncludes = qobject_cast<QListWidget*>(tc.Info["INCLUDES"]);
    for (int i = 0; i < listWidgetIncludes->count(); ++i)
        ti.includes.push_back(listWidgetIncludes->item(i)->text().toStdString());

    return true;
}

bool MainWindow::ReadCurrentFileInfo()
{
    parameters_compiler::info_info iim{};
    if (!ReadCurrentMainInfo(iim))
        return false;
    fileInfo_.info = iim;

    if (HaveCurrentParameter("Main"))
    {
        parameters_compiler::parameter_info pim{};
        if (!ReadCurrentParameter("Main", pim))
            return false;

        if (!parameters_compiler::helper::set_parameter_info(fileInfo_, "Main", pim))
            return false;
    }

    for (const auto& type : parameters_compiler::helper::get_user_type_names(fileInfo_))
    {
        parameters_compiler::type_info tit{};
        if (!ReadCurrentTypeInfo(QString::fromStdString(type), tit))
            return false;

        if (!parameters_compiler::helper::set_type_info(fileInfo_, tit.name, tit, true))
            return false;

        if (HaveCurrentParameter(QString::fromStdString(type)))
        {
            parameters_compiler::parameter_info pit;
            if (!ReadCurrentParameter(QString::fromStdString(tit.name), pit))
                return false;

            if (!parameters_compiler::helper::set_parameter_info(fileInfo_, tit.name, pit))
                return false;
        }
    }

    return true;
}

bool MainWindow::FillPropertyTypeNames()
{
    TabControls& tcm = GetTabControls("Main");

    QComboBox* comboBoxMain = qobject_cast<QComboBox*>(tcm.Properties["TYPE"]);
    QString textMain = comboBoxMain->currentText();
    comboBoxMain->clear();
    for (const auto& s : parameters_compiler::helper::get_property_type_names(fileInfo_))
        comboBoxMain->addItem(QString::fromStdString(s));
    comboBoxMain->setCurrentText(textMain);

    for (int i = 0; i < tabs_.size(); i++)
    {
        TabControls& tct = tabs_[i];

        QComboBox* comboBoxPropertyType = qobject_cast<QComboBox*>(tct.Properties["TYPE"]);
        QString textPropertyType = comboBoxPropertyType->currentText();
        comboBoxPropertyType->clear();
        for (const auto& s : parameters_compiler::helper::get_property_type_names(fileInfo_))
            comboBoxPropertyType->addItem(QString::fromStdString(s));
        comboBoxPropertyType->setCurrentText(textPropertyType);
    }

    return true;
}

bool MainWindow::RenamePropertyTypeNames(QString oldName, QString newName)
{
    TabControls& tcm = GetTabControls("Main");

    QString arrayOldName = QString("array<%1>").arg(oldName);
    QString arrayNewName = QString("array<%1>").arg(newName);

    QComboBox* comboBoxMain = qobject_cast<QComboBox*>(tcm.Properties["TYPE"]);
    QString textMain = comboBoxMain->currentText();
    comboBoxMain->clear();
    for (const auto& s : parameters_compiler::helper::get_property_type_names(fileInfo_))
        comboBoxMain->addItem(QString::fromStdString(s));
    if (textMain == oldName)
        comboBoxMain->setCurrentText(newName);
    else if (textMain == arrayOldName)
        comboBoxMain->setCurrentText(arrayNewName);
    else
        comboBoxMain->setCurrentText(textMain);

    for (int i = 0; i < tabs_.size(); i++)
    {
        TabControls& tct = tabs_[i];

        QComboBox* comboBoxPropertyType = qobject_cast<QComboBox*>(tct.Properties["TYPE"]);
        QString textPropertyType = comboBoxPropertyType->currentText();
        comboBoxPropertyType->clear();
        for (const auto& s : parameters_compiler::helper::get_property_type_names(fileInfo_))
            comboBoxPropertyType->addItem(QString::fromStdString(s));
        if (textPropertyType == oldName)
            comboBoxPropertyType->setCurrentText(newName);
        if (textPropertyType == arrayOldName)
            comboBoxPropertyType->setCurrentText(arrayNewName);
        else
            comboBoxPropertyType->setCurrentText(textPropertyType);
    }

    return true;
}

QWidget* MainWindow::CreatePropertiesWidget(QString type)
{
    QGridLayout* gridLayoutProperties = new QGridLayout;

    int index = 0;
    AddLineEditProperty(gridLayoutProperties, "NAME", index++, type, MainWindow::ControlsGroup::Properties, true);
    AddComboBoxPropertyType(gridLayoutProperties, "TYPE", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "DISPLAY_NAME", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddPlainTextEditProperty(gridLayoutProperties, "DESCRIPTION", index++, type, MainWindow::ControlsGroup::Properties);
    AddCheckBoxProperty(gridLayoutProperties, "REQUIRED", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "DEFAULT", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddLineEditProperty(gridLayoutProperties, "HINT", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (base)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddLineEditProperty(gridLayoutProperties, "MAX", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddListProperty(gridLayoutProperties, "SET", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (array)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN_COUNT", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddLineEditProperty(gridLayoutProperties, "MAX_COUNT", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddListProperty(gridLayoutProperties, "SET_COUNT", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (unit)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "CATEGORY", index++, type, MainWindow::ControlsGroup::Properties, false);
    AddListProperty(gridLayoutProperties, "IDS", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (path)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MAX_LENGTH", index++, type, MainWindow::ControlsGroup::Properties, false);

    gridLayoutProperties->setRowStretch(gridLayoutProperties->rowCount(), 1);

    QWidget* widgetSplitterProperties = new QWidget;
    widgetSplitterProperties->setLayout(gridLayoutProperties);

    QScrollArea* scrollAreaProperties = new QScrollArea;
    scrollAreaProperties->setWidget(widgetSplitterProperties);
    scrollAreaProperties->setWidgetResizable(true);
    scrollAreaProperties->setFrameStyle(0);

    return scrollAreaProperties;
}

QWidget* MainWindow::CreateListControlWidget(int buttonSize, QString type, ControlsGroup group, QString name, QString toolTipBase)
{
    QHBoxLayout* hBoxLayoutPropertyListButtons = new QHBoxLayout;
    hBoxLayoutPropertyListButtons->setMargin(0);

    QToolButton* toolButtonPropertyListAdd = new QToolButton;
    toolButtonPropertyListAdd->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListAdd->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListAdd->setIcon(QIcon(":/images/plus.png"));
    toolButtonPropertyListAdd->setProperty("type", type);
    toolButtonPropertyListAdd->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListAdd->setProperty("name", name);
    toolButtonPropertyListAdd->setProperty("action", "add");
    toolButtonPropertyListAdd->setToolTip(QString::fromLocal8Bit("Добавить %1").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListAdd);
    connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);

    QToolButton* toolButtonPropertyListRemove = new QToolButton;
    toolButtonPropertyListRemove->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListRemove->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListRemove->setIcon(QIcon(":/images/minus.png"));
    toolButtonPropertyListRemove->setProperty("type", type);
    toolButtonPropertyListRemove->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListRemove->setProperty("name", name);
    toolButtonPropertyListRemove->setProperty("action", "remove");
    toolButtonPropertyListRemove->setToolTip(QString::fromLocal8Bit("Удалить %1").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListRemove);
    connect(toolButtonPropertyListRemove, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);

    QToolButton* toolButtonPropertyListUp = new QToolButton;
    toolButtonPropertyListUp->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListUp->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListUp->setIcon(QIcon(":/images/up.png"));
    toolButtonPropertyListUp->setProperty("type", type);
    toolButtonPropertyListUp->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListUp->setProperty("name", name);
    toolButtonPropertyListUp->setProperty("action", "up");
    toolButtonPropertyListUp->setToolTip(QString::fromLocal8Bit("Поднять %1 в списке").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListUp);
    connect(toolButtonPropertyListUp, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);

    QToolButton* toolButtonPropertyListDown = new QToolButton;
    toolButtonPropertyListDown->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListDown->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListDown->setIcon(QIcon(":/images/down.png"));
    toolButtonPropertyListDown->setProperty("type", type);
    toolButtonPropertyListDown->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListDown->setProperty("name", name);
    toolButtonPropertyListDown->setProperty("action", "down");
    toolButtonPropertyListDown->setToolTip(QString::fromLocal8Bit("Опустить %1 в списке").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListDown);
    connect(toolButtonPropertyListDown, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);

    hBoxLayoutPropertyListButtons->addStretch();

    QFrame* widgetPropertyListButtons = new QFrame;
    widgetPropertyListButtons->setLayout(hBoxLayoutPropertyListButtons);
    widgetPropertyListButtons->setFrameShape(QFrame::NoFrame);

    return widgetPropertyListButtons;
}

QWidget* MainWindow::CreateTypeTabWidget(QString type)
{
    QWidget* widgetTabProperties = new QWidget;

    QWidget* widgetSplitterInfo = CreateTypeTabInfoWidget(type);
    QWidget* widgetSplitterPropertyList = CreatePropertyListWidget(type);
    QWidget* widgetSplitterProperties = CreatePropertiesWidget(type);

    AddGroupWidget(widgetSplitterInfo, "INFO_GROUP", type, ControlsGroup::Info);
    AddGroupWidget(widgetSplitterPropertyList, "PARAMETERS_GROUP", type, ControlsGroup::Parameters);
    AddGroupWidget(widgetSplitterProperties, "PROPERTIES_GROUP", type, ControlsGroup::Properties);

    QSplitter* tabHSplitter = new QSplitter(Qt::Horizontal);
    tabHSplitter->addWidget(widgetSplitterInfo);
    tabHSplitter->addWidget(widgetSplitterPropertyList);
    tabHSplitter->addWidget(widgetSplitterProperties);
    tabHSplitter->setStretchFactor(0, 1);
    tabHSplitter->setStretchFactor(1, 0);
    tabHSplitter->setStretchFactor(2, 1);
    widgetSplitterProperties->setEnabled(false);

    QVBoxLayout* vBoxLayoutSplitter = new QVBoxLayout;
    vBoxLayoutSplitter->addWidget(tabHSplitter);
    widgetTabProperties->setLayout(vBoxLayoutSplitter);

    return widgetTabProperties;
}

QWidget* MainWindow::CreateTypeTabInfoWidget(QString type)
{
    QGridLayout* gridLayoutInfo = new QGridLayout;

    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "TYPES", "font-weight: bold; font-size: 14px", index++);
    AddLineEditProperty(gridLayoutInfo, "NAME", index++, type, ControlsGroup::Info, false);
    AddComboBoxTypeType(gridLayoutInfo, "TYPE", index++, type, ControlsGroup::Info);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, type, ControlsGroup::Info);
    AddListProperty(gridLayoutInfo, "VALUES", index++, type, ControlsGroup::Info);
    AddListProperty(gridLayoutInfo, "INCLUDES", index++, type, ControlsGroup::Info);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

void MainWindow::on_ListControlClicked()
{
    QToolButton* tb = qobject_cast<QToolButton*>(sender());
    QString type = tb->property("type").toString();
    ControlsGroup group = static_cast<ControlsGroup>(tb->property("group").toInt());
    QString name = tb->property("name").toString();
    QString action = tb->property("action").toString();

    if (group == ControlsGroup::Parameters && name == "PARAMETERS" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add property", QString::fromLocal8Bit("Имя параметра:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (parameters_compiler::helper::get_parameter_info(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Параметр с именем %1 уже существует").arg(text));
            return;
        }

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PARAMETERS"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        parameters_compiler::parameter_info pi{};
        pi.name = text.toStdString();
        pi.type = "string";
        pi.required = true;
        if (!parameters_compiler::helper::add_parameter_info(fileInfo_, type.toStdString(), pi))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Parameters && name == "PARAMETERS" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PARAMETERS"]);
        if (!listWidget->currentItem())
            return;
        QString propertyName = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить свойство %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters_compiler::helper::remove_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Parameters && name == "PARAMETERS" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PARAMETERS"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString propertyName = listWidget->currentItem()->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Parameters && name == "PARAMETERS" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PARAMETERS"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString propertyName = listWidget->currentItem()->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "VALUES" && action == "add")
    {
        bool ok;
        QString textName = QInputDialog::getText(this, "Add value", QString::fromLocal8Bit("Имя:"), QLineEdit::Normal, "", &ok);
        if (!ok || textName.isEmpty())
            return;

        // Validate
        if (parameters_compiler::helper::have_info_value(fileInfo_, type.toStdString(), textName.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение с именем %1 уже существует").arg(textName));
            return;
        }

        QString textValue = QInputDialog::getText(this, "Add value", QString::fromLocal8Bit("Значение:"), QLineEdit::Normal, "", &ok);
        if (!ok || textValue.isEmpty())
            return;

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["VALUES"]);
        listWidget->addItem(new QListWidgetItem(textName + " -> " + textValue));

        // Add to fileInfo_
        if (!parameters_compiler::helper::add_info_value(fileInfo_, type.toStdString(), textName.toStdString(), textValue.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "VALUES" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["VALUES"]);
        if (!listWidget->currentItem())
            return;
        QString propertyName = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить значение %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        const auto s = propertyName.split(" -> ");
        if (s.size() < 1)
            return;
        if (!parameters_compiler::helper::remove_info_value(fileInfo_, type.toStdString(), s[0].toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "VALUES" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["VALUES"]);
        if (!listWidget->currentItem())
            return;
        QString propertyName = listWidget->currentItem()->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        const auto s = propertyName.split(" -> ");
        if (s.size() < 1)
            return;
        if (!parameters_compiler::helper::move_info_value(fileInfo_, type.toStdString(), s[0].toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "VALUES" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["VALUES"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString propertyName = listWidget->currentItem()->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        const auto s = propertyName.split(" -> ");
        if (s.size() < 1)
            return;
        if (!parameters_compiler::helper::move_info_value(fileInfo_, type.toStdString(), s[0].toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "INCLUDES" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add include", QString::fromLocal8Bit("Путь:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (parameters_compiler::helper::have_info_include(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["INCLUDES"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters_compiler::helper::add_info_include(fileInfo_, type.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "INCLUDES" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["INCLUDES"]);
        if (!listWidget->currentItem())
            return;
        QString propertyName = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить значение %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters_compiler::helper::remove_info_include(fileInfo_, type.toStdString(), propertyName.toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "INCLUDES" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["INCLUDES"]);
        if (!listWidget->currentItem())
            return;
        QString propertyName = listWidget->currentItem()->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_info_include(fileInfo_, type.toStdString(), propertyName.toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "INCLUDES" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["INCLUDES"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString propertyName = listWidget->currentItem()->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_info_include(fileInfo_, type.toStdString(), propertyName.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction", QString::fromLocal8Bit("Значение:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters_compiler::helper::have_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters_compiler::helper::add_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить значение %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters_compiler::helper::remove_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction", QString::fromLocal8Bit("Значение:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters_compiler::helper::have_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters_compiler::helper::add_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить значение %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters_compiler::helper::remove_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET_COUNT" && action == "add")
    {
        bool ok;
        int value = QInputDialog::getInt(this, "Add restriction", QString::fromLocal8Bit("Значение:"), 0, 0, 100000, 1, &ok);
        if (!ok) return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters_compiler::helper::have_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), QString("%1").arg(value).toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(value));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET_COUNT"]);
        listWidget->addItem(new QListWidgetItem(QString("%1").arg(value)));

        // Add to fileInfo_
        if (!parameters_compiler::helper::add_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), QString("%1").arg(value).toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET_COUNT" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET_COUNT"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить значение %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters_compiler::helper::remove_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET_COUNT" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET_COUNT"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET_COUNT" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET_COUNT"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "IDS" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction", QString::fromLocal8Bit("Значение:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters_compiler::helper::have_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["IDS"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters_compiler::helper::add_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "IDS" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["IDS"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("Вы действительно хотите удалить значение %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters_compiler::helper::remove_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "IDS" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["IDS"]);
        if (!listWidget->currentItem())
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->currentRow();
        if (currentRow <= 0)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow - 1, currentItem);
        listWidget->setCurrentRow(currentRow - 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "IDS" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["IDS"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString value = listWidget->currentItem()->text();

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        int currentRow = listWidget->row(listWidget->currentItem());
        if (currentRow >= listWidget->count() - 1)
            return;

        // Add to control
        QListWidgetItem* currentItem = listWidget->takeItem(currentRow);
        listWidget->insertItem(currentRow + 1, currentItem);
        listWidget->setCurrentRow(currentRow + 1);

        // Move in fileInfo_
        if (!parameters_compiler::helper::move_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
}

void MainWindow::on_CurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    bool modified = modified_;

    QListWidget* list = qobject_cast<QListWidget*>(sender());
    QString type = list->property("type").toString();

    parameters_compiler::parameter_info pic{};
    if (current != nullptr)
    {
        parameters_compiler::parameter_info* ppic = parameters_compiler::helper::get_parameter_info(fileInfo_, type.toStdString(), current->text().toStdString());
        if (ppic != nullptr) pic = *ppic;
    }

    if (previous != nullptr)
    {
        parameters_compiler::parameter_info pip{};
        if (!ReadCurrentParameter(type, pip))
            return;

        if (!parameters_compiler::helper::set_parameter_info(fileInfo_, type.toStdString(), pip))
            return;
    }

    TabControls& tc = GetTabControls(type);

    qobject_cast<QLineEdit*>(tc.Properties["NAME"])->setText(QString::fromStdString(pic.name));
    qobject_cast<QComboBox*>(tc.Properties["TYPE"])->setCurrentText(QString::fromStdString(pic.type));
    qobject_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->setText(QString::fromStdString(pic.display_name));
    qobject_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->setPlainText(QString::fromStdString(pic.description));
    qobject_cast<QCheckBox*>(tc.Properties["REQUIRED"])->setChecked(pic.required);
    qobject_cast<QLineEdit*>(tc.Properties["DEFAULT"])->setText(QString::fromStdString(pic.default_));
    qobject_cast<QLineEdit*>(tc.Properties["HINT"])->setText(QString::fromStdString(pic.hint));

    qobject_cast<QLineEdit*>(tc.Properties["MIN"])->setText(QString::fromStdString(pic.restrictions.min));
    qobject_cast<QLineEdit*>(tc.Properties["MAX"])->setText(QString::fromStdString(pic.restrictions.max));
    QListWidget* listWidgetSet = qobject_cast<QListWidget*>(tc.Properties["SET"]);
    listWidgetSet->clear();
    for (const auto& s : pic.restrictions.set_)
        listWidgetSet->addItem(QString::fromStdString(s));

    qobject_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->setText(QString::fromStdString(pic.restrictions.min_count));
    qobject_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->setText(QString::fromStdString(pic.restrictions.max_count));
    QListWidget* listWidgetSetCount = qobject_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    listWidgetSetCount->clear();
    for (const auto& s : pic.restrictions.set_count)
        listWidgetSetCount->addItem(QString::fromStdString(s));

    qobject_cast<QLineEdit*>(tc.Properties["CATEGORY"])->setText(QString::fromStdString(pic.restrictions.category));
    QListWidget* listWidgetIds = qobject_cast<QListWidget*>(tc.Properties["IDS"]);
    listWidgetIds->clear();
    for (const auto& s : pic.restrictions.ids)
        listWidgetIds->addItem(QString::fromStdString(s));

    qobject_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->setText(QString::fromStdString(pic.restrictions.max_length));

    if (current == nullptr)
    {
        qobject_cast<QWidget*>(tc.Properties["PROPERTIES_GROUP"])->setEnabled(false);
    }
    else
    {
        qobject_cast<QWidget*>(tc.Properties["PROPERTIES_GROUP"])->setEnabled(true);
    }

    modified_ = modified;
    UpdateWindowTitle();
}

void MainWindow::on_TextChanged()
{
    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::on_StateChanged(int state)
{
    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::on_EditingFinished()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if (!lineEdit->isModified()) return; //!!!
    lineEdit->setModified(false);
    qDebug() << lineEdit->text();

    QString name = lineEdit->property("name").toString();
    MainWindow::ControlsGroup group = static_cast<MainWindow::ControlsGroup>(lineEdit->property("group").toInt());
    QString type = lineEdit->property("type").toString();

    TabControls& tc = GetTabControls(type);

    if (group == MainWindow::ControlsGroup::Info && name == "ID")
    {
        // Main ID
    }
    else if (group == MainWindow::ControlsGroup::Info && name == "NAME")
    {
        // Type NAME
        QString oldName = type;
        QString newName = lineEdit->text();

        if (parameters_compiler::helper::get_type_info(fileInfo_, newName.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Тип с именем %1 уже существует").arg(newName));
            lineEdit->setText(oldName);
            return;
        }

        // Update controls
        for (int i = 0; i < tabWidget_->count(); i++)
        {
            if (tabWidget_->tabText(i) == oldName)
            {
                tabWidget_->setTabText(i, newName);
                break;
            }
        }

        // Update fileInfo_
        if (!parameters_compiler::helper::rename_type(fileInfo_, oldName.toStdString(), newName.toStdString()))
            return;

        // Update controls
        RenamePropertyTypeNames(oldName, newName);

        // Update controls
        RenameTabControlsType(oldName, newName);
    }
    else if (group == MainWindow::ControlsGroup::Properties && name == "NAME")
    {
        // Property NAME
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
        if (listWidget->selectedItems().size() == 0) return; // !!!

        QString oldName = listWidget->selectedItems()[0]->text();
        QString newName = lineEdit->text();

        // Update controls
        for (int i = 0; i < listWidget->count(); ++i)
        {
            if (listWidget->item(i)->text() == newName && listWidget->selectedItems()[0] != listWidget->item(i))
            {
                QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Свойство с именем %1 уже существует").arg(newName));
                if (listWidget->selectedItems().size() > 0)
                    lineEdit->setText(oldName);
                return;
            }
        }
        listWidget->selectedItems()[0]->setText(newName);

        // Update fileInfo_
        if (!parameters_compiler::helper::rename_property(fileInfo_, type.toStdString(), oldName.toStdString(), newName.toStdString()))
            return;
    }

    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::on_CurrentIndexChanged(int index)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
    QString type = comboBox->property("type").toString();
    QString name = comboBox->property("name").toString();

    QString t = comboBox->itemText(index);
    if (t == "yml")
    {
        QList<QString> usedInTypes;

        // Check if not array type used
        for (auto& p : fileInfo_.parameters)
        {
            if (QString::fromStdString(p.type) == type)
            {
                usedInTypes.push_back("Main");
                break;
            }
        }
        for (auto& t : fileInfo_.types)
        {
            for (auto& p : t.parameters)
            {
                if (QString::fromStdString(p.type) == type)
                {
                    usedInTypes.push_back(QString::fromStdString(t.name));
                    break;
                }
            }
        }

        if (usedInTypes.size() > 0)
        {
            QString message = QString::fromLocal8Bit("Тип %1 используется для параметра в другом типе,\nно для типов yml допускается использование только в массивах array<%1>. Типы:\n").arg(type);
            for (const auto& s : usedInTypes)
                message += s + "\n";
            QMessageBox::StandardButton resBtn = QMessageBox::critical(this, "parameters_composer", message);

            comboBox->setCurrentText("enum"); // !!! need get value from fileInfo_
            return;
        }
    }

    parameters_compiler::type_info tit{};
    if (!ReadCurrentTypeInfo(type, tit))
        return;

    auto ti = parameters_compiler::helper::get_type_info(fileInfo_, type.toStdString());
    ti->type = tit.type;
    //if (!parameters_compiler::helper::set_type_info(fileInfo_, type.toStdString(), tit, true))
    //    return;

    FillPropertyTypeNames();

    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::Update()
{
    UpdateMain();
    for (const auto& type : parameters_compiler::helper::get_user_type_names(fileInfo_))
        UpdateType(QString::fromStdString(type));
}

void MainWindow::UpdateMain()
{
    TabControls& tc = GetTabControls("Main");

    qobject_cast<QLineEdit*>(tc.Info["ID"])->setText(QString::fromStdString(fileInfo_.info.id));
    qobject_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->setText(QString::fromStdString(fileInfo_.info.display_name));
    qobject_cast<QLineEdit*>(tc.Info["CATEGORY"])->setText(QString::fromStdString(fileInfo_.info.category));
    qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString::fromStdString(fileInfo_.info.description));
    qobject_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->setText(QString::fromStdString(fileInfo_.info.pictogram));
    qobject_cast<QLineEdit*>(tc.Info["HINT"])->setText(QString::fromStdString(fileInfo_.info.hint));
    qobject_cast<QLineEdit*>(tc.Info["AUTHOR"])->setText(QString::fromStdString(fileInfo_.info.author));
    qobject_cast<QLineEdit*>(tc.Info["WIKI"])->setText(QString::fromStdString(fileInfo_.info.wiki));
    qobject_cast<QLineEdit*>(tc.Info["WIKI"])->setCursorPosition(0);

    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
    for (const auto& pi : fileInfo_.parameters)
        listWidget->addItem(QString::fromStdString(pi.name));
    if (listWidget->count() > 0)
        listWidget->setCurrentRow(0);
}

void MainWindow::UpdateType(QString type)
{
    TabControls& tc = GetTabControls(type);
    const auto& ti = parameters_compiler::helper::get_type_info(fileInfo_, type.toStdString());

    qobject_cast<QLineEdit*>(tc.Info["NAME"])->setText(QString::fromStdString(ti->name));
    qobject_cast<QComboBox*>(tc.Info["TYPE"])->setCurrentText(QString::fromStdString(ti->type));
    //QString textTypeType = qobject_cast<QComboBox*>(tc.Info["TYPE"])->currentText();
    qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString::fromStdString(ti->description));

    QListWidget* listWidgetValues = qobject_cast<QListWidget*>(tc.Info["VALUES"]);
    listWidgetValues->clear();
    for (const auto& v : ti->values)
        listWidgetValues->addItem(QString::fromStdString(v.first) + " -> " + QString::fromStdString(v.second));

    QListWidget* listWidgetIncludes = qobject_cast<QListWidget*>(tc.Info["INCLUDES"]);
    listWidgetIncludes->clear();
    for (const auto& v : ti->includes)
        listWidgetIncludes->addItem(QString::fromStdString(v));

    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
    listWidget->clear();
    for (const auto& pi : ti->parameters)
        listWidget->addItem(QString::fromStdString(pi.name));
    if (listWidget->count() > 0)
        listWidget->setCurrentRow(0);
}

void MainWindow::UpdateWindowTitle()
{
    QString title = "parameters_composer - Untitled";
    if (currentFileName_ != "")
        title = QString("parameters_composer - %1").arg(currentFileName_);
    if (modified_)
        title += "*";
    setWindowTitle(title);
}

void MainWindow::SaveAs()
{
    QFileDialog dialog(this);
    dialog.setNameFilters({ "Parameters Compiler YAML Files (*.yml *.yaml)", "Parameters Compiler JSON Files (*.json)" });
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("yml");

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();
    if (fileNames.size() <= 0)
        return;

    bool is_json = (dialog.selectedNameFilter() == "Parameters Compiler JSON Files (*.json)");
    QString fileName = fileNames[0];
    SaveAsInternal(fileName, is_json, false);
}

bool MainWindow::SaveAsInternal(QString fileName, bool is_json, bool is_temp)
{
    if (!ReadCurrentFileInfo())
        return false;

    parameters_compiler::file_info fi = fileInfo_;

    std::string message;
    if (!parameters_compiler::helper::validate(fi, message))
    {
        QMessageBox::critical(this, "Validate error", QString::fromLocal8Bit(message.c_str()));
        return false;
    }

    bool have_type_loop = false;
    if (!parameters_compiler::helper::rearrange_types(fi, have_type_loop))
    {
        QMessageBox::critical(this, "Rearrange error", QString::fromLocal8Bit("Ошибка переупорядочивания пользовательских типов перед сохранением"));
        return false;
    }
    else if (have_type_loop)
    {
        QMessageBox::warning(this, "Rearrange", QString::fromLocal8Bit("Обнаружена циклицеская зависимость в типах.\nФайл будет сохранен, но эта логическая ошибка требует исправления!"));
    }

    if (is_json)
    {
        if (!json::writer::write(fileName.toStdString(), fi))
            return false;
    }
    else
    {
        if (!yaml::writer::write(fileName.toStdString(), fi))
            return false;
    }

    if (!is_temp)
    {
        currentFileName_ = fileName;
        modified_ = false;
        is_json_ = is_json;
        UpdateWindowTitle();
    }

    return true;
}

void MainWindow::on_FocusChanged(QObject* sender, bool focus)
{
    QString type = sender->property("type").toString();
    ControlsGroup group = static_cast<ControlsGroup>(sender->property("group").toInt());
    QString name = sender->property("name").toString();

    parameters_compiler::struct_types st;
    if (type == "Main")
    {
        if (group == ControlsGroup::Info)
            st = parameters_compiler::struct_types::info_info;
        else if (group == ControlsGroup::Properties)
            st = parameters_compiler::struct_types::parameter_info;
        else if (group == ControlsGroup::Parameters)
            st = parameters_compiler::struct_types::file_info;
    }
    else
    {
        if (group == ControlsGroup::Info)
            st = parameters_compiler::struct_types::type_info;
        else if (group == ControlsGroup::Properties)
            st = parameters_compiler::struct_types::parameter_info;
        else if (group == ControlsGroup::Parameters)
            st = parameters_compiler::struct_types::type_info;
    }

    QString text = QString::fromLocal8Bit(parameters_compiler::helper::get_hint_html(st, name.toStdString()).c_str());

    // restrictions_info is a part of parameter_info, but on gui we not divide it
    if (text == "" && st == parameters_compiler::struct_types::parameter_info)
        text = QString::fromLocal8Bit(parameters_compiler::helper::get_hint_html(parameters_compiler::struct_types::restrictions_info, name.toStdString()).c_str());

    plainTextEditHint_->clear();
    plainTextEditHint_->appendHtml(QString("<p style='font-weight: bold; font-size: 14px;'>%1</p>").arg(name));
    plainTextEditHint_->appendHtml(text);
}
