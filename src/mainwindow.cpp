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
#include <QDesktopServices>
#include <QSettings>

#include <fstream>
#include "parameters.h"

#include "refactoring_helper.h"
#include "base64.h"
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
            QString::fromLocal8Bit("�� ������������� ������ �����?\n��� ������������� ��������� ����� ��������!"), QMessageBox::No | QMessageBox::Yes);
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
            QString::fromLocal8Bit("�� ������������� ������ ������� ����� ����?\n��� ������������� ��������� ����� ��������!"), QMessageBox::No | QMessageBox::Yes);
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
            QString::fromLocal8Bit("�� ������������� ������ �����?\n��� ������������� ��������� ����� ��������!"), QMessageBox::No | QMessageBox::Yes);
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
    if (modified_)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString::fromLocal8Bit("�� ������������� ������ ������ ���������?\n��� �������� � �������� ������ �����.\n��� ������������� ��������� ����� ��������!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;
    }

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
            QString::fromLocal8Bit("�� ������������� ������ ������� ����?\n��� ������������� ��������� ����� ��������!"), QMessageBox::No | QMessageBox::Yes);
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
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ������").arg(fileName));
        RemoveRecent(fileName);
        return false;
    }

    parameters::file_info fi{};
    parameters::yaml::parser yp(false);
    if (!yp.parse(fileName.toStdString(), fi))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("������ ������� ����� %1").arg(fileName));
        return false;
    }

    while (tabWidget_->count() > 1)
        tabWidget_->removeTab(1);

    TabControls& tc = GetTabControls("Main");
    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
    listWidget->clear();

    fileInfo_ = fi;

    for (const auto& type : parameters::helper::file::get_user_types(fileInfo_))
    {
        QWidget* widgetTabType = CreateTypeTabWidget(QString::fromStdString(type));
        tabWidget_->addTab(widgetTabType, QString::fromStdString(type));
        UpdateType(QString::fromStdString(type));
    }
    FillPropertyTypeNames();
    UpdateMain();

    currentFileName_ = fileName;
    modified_ = false;
    UpdateWindowTitle();
    AddRecent(fileName);

    return true;
}

bool MainWindow::ApplyInternal(QString cmakeFilePath)
{
    qDebug() << "Apply to " << cmakeFilePath;

    if (!QFile::exists(cmakeFilePath))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ������").arg(cmakeFilePath));
        return false;
    }

    QString cmakeText;
    if (!refactoring::helper::read_file(cmakeFilePath, "UTF-8", cmakeText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ��������").arg(cmakeFilePath));
        return false;
    }

    QString targetName;
    if (!refactoring::helper::get_single_value(cmakeText, R"wwww((SET|set)(\s+|)\((\s+|)TARGET_NAME\s+(?<value>\w*))wwww", targetName))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("TARGET_NAME �� ������ � %1").arg(cmakeText));
        return false;
    }

    if (refactoring::helper::check_cmake_is_patched(cmakeText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 ��� ���������").arg(cmakeFilePath));
        return false;
    }


    QString cmakePatched;
    if (!refactoring::helper::patch_cmake(cmakeText, cmakePatched))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ���������").arg(cmakeFilePath));
        return false;
    }

    QString paramsPath = QFileInfo(cmakeFilePath).absoluteDir().filePath("params");
    QString targetFileName = QString("%1%2").arg(targetName, ".yml");
    QString paramsFilePath = QDir(paramsPath).filePath(targetFileName);

    if (!QFile::exists(paramsPath))
    {
        if (!QDir().mkpath(paramsPath))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�� ������� ������� ����� %1").arg(paramsPath));
            return false;
        }
    }
    else if (QFile::exists(paramsFilePath))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 ��� ����������").arg(paramsFilePath));
        return false;
    }

    QString libClassFileName = QFileInfo(cmakeFilePath).absoluteDir().filePath("lib_class_name.h");
    if (!QFile::exists(libClassFileName))
    {
        libClassFileName = QFileInfo(cmakeFilePath).absoluteDir().filePath("src/lib_class_name.h");
        if (!QFile::exists(libClassFileName))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� lib_class_name.h ��� src/lib_class_name.h �� ������"));
            return false;
        }
    }

    QString libClassText;
    if (!refactoring::helper::read_file(libClassFileName, "CP-1251", libClassText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ��������").arg(libClassFileName));
        return false;
    }

    QString className;
    if (!refactoring::helper::get_single_value(libClassText, R"wwww(#define(\s+)LIBRARY_CLASS_NAME(\s+)\w+::(?<value>\w*))wwww", className))
    {
        if (!refactoring::helper::get_single_value(libClassText, R"wwww(#define(\s+)LIBRARY_CLASS_NAME(\s+)(?<value>\w*))wwww", className))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("LIBRARY_CLASS_NAME �� ������ � %1").arg(libClassFileName));
            return false;
        }
    }

    QString includeFile;
    if (!refactoring::helper::get_single_value(libClassText, R"wwww(^#include "(?<value>.*)")wwww", includeFile))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("#include �� ������ � %1").arg(libClassFileName));
        return false;
    }

    QString includeFilePath = QFileInfo(cmakeFilePath).absoluteDir().filePath(includeFile);
    if (!QFile::exists(includeFilePath))
    {
        includeFilePath = QFileInfo(cmakeFilePath).absoluteDir().filePath(QString("src/%1").arg(includeFile));
        if (!QFile::exists(includeFilePath))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ������").arg(includeFile));
            return false;
        }
    }

    QString includeText;
    if (!refactoring::helper::read_file(includeFilePath, "CP-1251", includeText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ��������").arg(includeFilePath));
        return false;
    }

    if (refactoring::helper::check_include_is_patched(includeText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 ��� ���������").arg(includeFilePath));
        return false;
    }

    QString includePatched;
    if (!refactoring::helper::patch_include(includeText, targetName, className, includePatched))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ���������").arg(includeFilePath));
        return false;
    }

    QFileInfo includeFileInfo(includeFilePath);
    QString cppFilePath = includeFileInfo.path() + "/" + includeFileInfo.completeBaseName() + ".cpp";

    QString cppText;
    if (!refactoring::helper::read_file(cppFilePath, "CP-1251", cppText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ��������").arg(cppFilePath));
        return false;
    }

    if (refactoring::helper::check_cpp_is_patched(cppText))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 ��� ���������").arg(cppFilePath));
        return false;
    }

    QString cppPatched;
    if (!refactoring::helper::patch_cpp(cppText, className, cppPatched))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� ���������").arg(cppFilePath));
        return false;
    }

    QStringList parameters;
    if (!refactoring::helper::get_parameters(cppText, className, parameters))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("������ ���������� �� ������� �� ����� %1").arg(cppFilePath));
        return false;
    }




    // ������ CMake
    if (!refactoring::helper::write_file(cmakeFilePath, "UTF-8", cmakePatched))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� �������").arg(cmakeFilePath));
        return false;
    }

    // ������ include
    if (!refactoring::helper::write_file(includeFilePath, "CP-1251", includePatched))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� �������").arg(includeFilePath));
        return false;
    }

    // ������ cpp
    if (!refactoring::helper::write_file(cppFilePath, "CP-1251", cppPatched))
    {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("���� %1 �� �������").arg(cppFilePath));
        return false;
    }

    // ��������� ��� � ���������
    parameters::file_info fi{};
    fi.info.id = targetName.toStdString();
    for (const auto& p : parameters)
    {
        QString name = refactoring::helper::get_parameter_name(cppText, p);
        QString description = refactoring::helper::get_parameter_description(cppText, p);

        parameters::parameter_info pi{};
        pi.name = name == "" ? p.toStdString() : name.toStdString();
        pi.required = true;
        pi.type = "string";
        pi.description = description.toStdString();
        fi.parameters.push_back(pi);
    }

    // ��������� ���� YAML �� ����
    parameters::yaml::writer writer{};
    writer.write(paramsFilePath.toStdString(), fi);
    if (!OpenFileInternal(paramsFilePath, false))
        return false; // Message inside OpenFileInternal

    QMessageBox::information(this, "Ok", QString::fromLocal8Bit("��������� ������ �������!\n�������������� ��������� ����� � ������������� build.py.\n������� �������� ��� � �������� ������ �����."));

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
        QAction* recentAct = new QAction(QString::fromLocal8Bit("<������ ����>"), this);
        recentAct->setStatusTip(QString::fromLocal8Bit("������ ����"));
        recentMenu_->addAction(recentAct);
    }
    else
    {
        for (int i = 0; i < recent_count; i++)
        {
            QString name = QString("recent/filename_%1").arg(i);
            QString path = app_settings.value(name, "").toString();
            QAction* recentAct = new QAction(path, this);
            recentAct->setStatusTip(QString::fromLocal8Bit("������� ���� %1").arg(path));
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
            QString::fromLocal8Bit("�� ������������� ������ ��������� ����?\n���� ����� �����������!"), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        if (!ReadCurrentFileInfo())
            return;

        std::string message;
        if (!parameters::helper::file::validate(fileInfo_, message))
        {
            QMessageBox::critical(this, "Validate error", QString::fromLocal8Bit(message.c_str()));
            return;
        }

        bool have_type_loop = false;
        if (!parameters::helper::file::rearrange_types(fileInfo_, have_type_loop))
        {
            QMessageBox::critical(this, "Rearrange error", QString::fromLocal8Bit("������ ������������������ ���������������� ����� ����� �����������"));
            return;
        }
        else if (have_type_loop)
        {
            QMessageBox::warning(this, "Rearrange", QString::fromLocal8Bit("���������� ����������� ����������� � �����.\n���� ����� ��������, �� ��� ���������� ������ ������� �����������!"));
        }

        if (is_json_)
        {
            if (!parameters::json::writer::write(currentFileName_.toStdString(), fileInfo_))
                return;
        }
        else
        {
            parameters::yaml::writer writer{};
            if (!writer.write(currentFileName_.toStdString(), fileInfo_))
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
    QString text = QInputDialog::getText(this, "Add type", QString::fromLocal8Bit("��� ����:"), QLineEdit::Normal, "", &ok);
    if (!ok || text.isEmpty())
        return;

    for (const auto& ti : fileInfo_.types)
    {
        if (QString::fromStdString(ti.name) == text)
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("��� � ������ %1 ��� ����������").arg(text));
            return;
        }
    }

    QWidget* widgetTabType = CreateTypeTabWidget(text);
    tabWidget_->addTab(widgetTabType, text);
    tabWidget_->setCurrentWidget(widgetTabType);

    TabControls& tc = GetTabControls(text);
    qobject_cast<QLineEdit*>(tc.Info["NAME"])->setText(text);

    parameters::type_info ti{};
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
        QMessageBox::warning(this, "parameters_composer", QString::fromLocal8Bit("������� �������� �� ����� ���� �������"));
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        QString::fromLocal8Bit("�� ������������� ������ ������� ��� %1?").arg(name), QMessageBox::No | QMessageBox::Yes);
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
        QString message = QString::fromLocal8Bit("��� %1 ������������ � ������ �����:\n").arg(name);
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
        QMessageBox::information(this, "parameters_composer", QString::fromLocal8Bit("���� ������� �������������"));
}

bool MainWindow::Compile()
{
    QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
    QString ymlFilePath = QDir(workingDir).filePath("temp.yml");
    QString jsonFilePath = QDir(workingDir).filePath("temp.json");

    if (!SaveAsInternal(ymlFilePath, false, true))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("������ ��� ���������� ����� YAML"));
        return false;
    }

    if (!SaveAsInternal(jsonFilePath, true, true))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("������ ��� ���������� ����� JSON"));
        return false;
    }

    parameters::file_info fileInfoYml{};
    if (!parameters::yaml::parser::parse(ymlFilePath.toStdString(), true, fileInfoYml))
    {
        QMessageBox::critical(this, "Compile YAML", QString::fromLocal8Bit("������ ���������� ����� YAML"));
        return false;
    }

    parameters::file_info fileInfoJson{};
    if (!parameters::yaml::parser::parse(jsonFilePath.toStdString(), true, fileInfoJson))
    {
        QMessageBox::critical(this, "Compile YAML", QString::fromLocal8Bit("������ ���������� ����� YAML"));
        return false;
    }

    std::string message;
    if (!parameters::helper::file::compare(fileInfoYml, fileInfoJson, message))
    {
        QString e = QString::fromStdString(message);
        QString m = QString::fromLocal8Bit("����� YAML � JSON �� ���������, ������:\n%1").arg(e);
        QMessageBox::critical(this, "Compile YAML", m);
        return false;
    }

    std::list<std::string> code;
    if (!parameters::formatter::code_formatter::format(fileInfoYml, true, code))
    {
        QMessageBox::critical(this, "Compile YAML", QString::fromLocal8Bit("������ ������������ ������������� �����"));
        return false;
    }

    std::list<std::string> wiki;
    if (!parameters::formatter::wiki_formatter::format(fileInfoYml, true, wiki))
    {
        QMessageBox::critical(this, "Compile YAML", QString::fromLocal8Bit("������ ������������ ����� wiki"));
        return false;
    }

    std::list<std::string> html;
    if (!parameters::formatter::html_formatter::format(fileInfoYml, true, html))
    {
        QMessageBox::critical(this, "Compile YAML", QString::fromLocal8Bit("������ ������������ ����� html"));
        return false;
    }

    QString outputHeaderFilePath = QDir(workingDir).filePath("temp.yml.h");
    std::ofstream output_header_file(outputHeaderFilePath.toStdString());
    if (!output_header_file.is_open())
    {
        QString m = QString::fromLocal8Bit("������ ���������� ������������� �����:\n%1").arg(outputHeaderFilePath);
        QMessageBox::critical(this, "Compile YAML", m);
        return false;
    }
    for (const auto& s : code)
        output_header_file << s << std::endl;
    output_header_file.close();

    QString outputWikiFilePath = QDir(workingDir).filePath("temp.yml.txt");
    std::ofstream output_wiki_file(outputWikiFilePath.toStdString());
    if (!output_wiki_file.is_open())
    {
        QString m = QString::fromLocal8Bit("������ ���������� ����� wiki:\n%1").arg(outputWikiFilePath);
        QMessageBox::critical(this, "Compile YAML", m);
        return false;
    }
    for (const auto& s : wiki)
        output_wiki_file << s << std::endl;
    output_wiki_file.close();

    QString outputHtmlFilePath = QDir(workingDir).filePath("temp.yml.html");
    std::ofstream output_html_file(outputHtmlFilePath.toStdString());
    if (!output_html_file.is_open())
    {
        QString m = QString::fromLocal8Bit("������ ���������� ����� html:\n%1").arg(outputHtmlFilePath);
        QMessageBox::critical(this, "Compile YAML", m);
        return false;
    }
    for (const auto& s : html)
        output_html_file << s << std::endl;
    output_html_file.close();

    return true;
}

void MainWindow::on_ViewYaml_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.yml")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("���� temp.yml �� ������.\n��������� ����������!"));
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
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("���� temp.json �� ������.\n��������� ����������!"));
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
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("���� temp.yml.h �� ������.\n��������� ����������!"));
            return;
        }
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("temp.yml.h"))));
    }
}

void MainWindow::on_ViewWiki_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.yml.txt")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("���� temp.yml.txt �� ������.\n��������� ����������!"));
            return;
        }
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("temp.yml.txt"))));
    }
}

void MainWindow::on_ViewHtml_action()
{
    if (Compile())
    {
        QString workingDir = QDir(QCoreApplication::applicationDirPath()).filePath("parameters_compiler");
        if (!QFileInfo::exists(QDir(workingDir).filePath("temp.yml.html")))
        {
            QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("���� temp.html �� ������.\n��������� ����������!"));
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
    if (!QFileInfo::exists(QDir(workingDir).filePath("parameters_compiler.docx")))
    {
        QMessageBox::critical(this, "parameters_composer", QString::fromLocal8Bit("���� parameters_compiler.docx �� ������"));
        return;
    }
    QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(QDir(workingDir).filePath("parameters_compiler.docx"))));
}

void MainWindow::on_Recent_action()
{
    QAction* act = qobject_cast<QAction*>(sender());
    OpenFile(act->text());
}

void MainWindow::CreateMenu()
{
    QAction* newAct = new QAction(QString::fromLocal8Bit("�������"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(QString::fromLocal8Bit("������� ����� ����"));
    connect(newAct, &QAction::triggered, this, &MainWindow::on_NewFile_action);

    QAction* openAct = new QAction(QString::fromLocal8Bit("�������"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(QString::fromLocal8Bit("������� ����"));
    connect(openAct, &QAction::triggered, this, &MainWindow::on_OpenFile_action);

    QAction* saveAct = new QAction(QString::fromLocal8Bit("���������"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(QString::fromLocal8Bit("��������� ����"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::on_SaveFile_action);

    QAction* saveAsAct = new QAction(QString::fromLocal8Bit("��������� ���..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(QString::fromLocal8Bit("��������� ���� ���..."));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::on_SaveAsFile_action);

    QAction* applyAct = new QAction(QString::fromLocal8Bit("��������"), this);
    applyAct->setShortcuts(QKeySequence::SaveAs);
    applyAct->setStatusTip(QString::fromLocal8Bit("�������� � ������"));
    connect(applyAct, &QAction::triggered, this, &MainWindow::on_Apply_action);

    QAction* quitAct = new QAction(QString::fromLocal8Bit("�����"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(QString::fromLocal8Bit("����� �� ����������"));
    connect(quitAct, &QAction::triggered, this, &MainWindow::on_Quit_action);

    QMenu* fileMenu = menuBar()->addMenu(QString::fromLocal8Bit("����"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(applyAct);
    fileMenu->addSeparator();
    recentMenu_ = fileMenu->addMenu(QString::fromLocal8Bit("�������� �����"));
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    QAction* addTypeAct = new QAction(QString::fromLocal8Bit("��������"), this);
    addTypeAct->setStatusTip(QString::fromLocal8Bit("�������� ���"));
    connect(addTypeAct, &QAction::triggered, this, &MainWindow::on_AddType_action);

    QAction* removeTypeAct = new QAction(QString::fromLocal8Bit("�������"), this);
    removeTypeAct->setStatusTip(QString::fromLocal8Bit("������� ���"));
    connect(removeTypeAct, &QAction::triggered, this, &MainWindow::on_RemoveType_action);

    QMenu* typeMenu = menuBar()->addMenu(QString::fromLocal8Bit("����"));
    typeMenu->addAction(addTypeAct);
    typeMenu->addAction(removeTypeAct);

    QAction* compileAct = new QAction(QString::fromLocal8Bit("���� ����������"), this);
    compileAct->setStatusTip(QString::fromLocal8Bit("������������� ��������� ����"));
    connect(compileAct, &QAction::triggered, this, &MainWindow::on_Compile_action);

    QAction* viewYamlAct = new QAction(QString::fromLocal8Bit("�������� YAML"), this);
    viewYamlAct->setStatusTip(QString::fromLocal8Bit("��������������� �������� YAML �����"));
    connect(viewYamlAct, &QAction::triggered, this, &MainWindow::on_ViewYaml_action);

    QAction* viewJsonAct = new QAction(QString::fromLocal8Bit("�������� JSON"), this);
    viewJsonAct->setStatusTip(QString::fromLocal8Bit("��������������� �������� JSON �����"));
    connect(viewJsonAct, &QAction::triggered, this, &MainWindow::on_ViewJson_action);

    QAction* viewCodeAct = new QAction(QString::fromLocal8Bit("�������� ����"), this);
    viewCodeAct->setStatusTip(QString::fromLocal8Bit("��������������� �������� .h �����"));
    connect(viewCodeAct, &QAction::triggered, this, &MainWindow::on_ViewCode_action);

    QAction* viewWikiAct = new QAction(QString::fromLocal8Bit("�������� WIKI"), this);
    viewWikiAct->setStatusTip(QString::fromLocal8Bit("��������������� �������� WIKI �����"));
    connect(viewWikiAct, &QAction::triggered, this, &MainWindow::on_ViewWiki_action);

    QAction* viewHtmlAct = new QAction(QString::fromLocal8Bit("�������� HTML"), this);
    viewHtmlAct->setStatusTip(QString::fromLocal8Bit("��������������� �������� HTML �����"));
    connect(viewHtmlAct, &QAction::triggered, this, &MainWindow::on_ViewHtml_action);

    QAction* openFolderAct = new QAction(QString::fromLocal8Bit("�������� � �����"), this);
    openFolderAct->setStatusTip(QString::fromLocal8Bit("�������� ����� � �����"));
    connect(openFolderAct, &QAction::triggered, this, &MainWindow::on_OpenFolder_action);

    QMenu* compileMenu = menuBar()->addMenu(QString::fromLocal8Bit("����������"));
    compileMenu->addAction(compileAct);
    compileMenu->addSeparator();
    compileMenu->addAction(viewYamlAct);
    compileMenu->addAction(viewJsonAct);
    compileMenu->addAction(viewCodeAct);
    compileMenu->addAction(viewWikiAct);
    compileMenu->addAction(viewHtmlAct);
    compileMenu->addSeparator();
    compileMenu->addAction(openFolderAct);

    QAction* helpAct = new QAction(QString::fromLocal8Bit("������� ��������"), this);
    helpAct->setStatusTip(QString::fromLocal8Bit("������� �������� ������� ���������������� ������"));
    connect(helpAct, &QAction::triggered, this, &MainWindow::on_Help_action);

    QMenu* helpMenu = menuBar()->addMenu(QString::fromLocal8Bit("������"));
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

    //focusFilter_ = new FocusFilter;
    //connect(focusFilter_, &FocusFilter::onFocusChanged, this, &MainWindow::on_FocusChanged);

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
    //AddLineEditProperty(gridLayoutInfo, "PICTOGRAM", index++, type, ControlsGroup::Info, false);
    AddPictogramProperty(gridLayoutInfo, "PICTOGRAM", index++, type, ControlsGroup::Info);
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

    QWidget* widgetPropertyListButtons = CreateListControlWidget(32, type, ControlsGroup::Parameters, "PARAMETERS", QString::fromLocal8Bit("��������"), true);

    QListWidget* listWidget = new QListWidget;
    listWidget->setProperty("name", "PARAMETERS");
    listWidget->setProperty("group", static_cast<int>(ControlsGroup::Parameters));
    listWidget->setProperty("type", type);
    //listWidget->installEventFilter(focusFilter_);

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
    //lineEdit->installEventFilter(focusFilter_);

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
    //plainTextEdit->installEventFilter(focusFilter_);

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
    //checkBox->installEventFilter(focusFilter_);

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
    QWidget* widgetPropertiesRestrictionsSetButtons = CreateListControlWidget(24, type, group, name, QString::fromLocal8Bit("�������� %1").arg(name), false);
    QListWidget* listWidget = new QListWidget;
    listWidget->setProperty("name", name);
    listWidget->setProperty("group", static_cast<int>(group));
    listWidget->setProperty("type", type);
    //listWidget->installEventFilter(focusFilter_);

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
    //comboBox->installEventFilter(focusFilter_);

    for (const auto& s : parameters::helper::file::get_parameter_types(fileInfo_))
        comboBox->addItem(QString::fromStdString(s));

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_CurrentIndexChanged);
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
    //comboBox->installEventFilter(focusFilter_);

    for (const auto& s : parameters::helper::file::get_type_types())
        comboBox->addItem(QString::fromStdString(s));

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_CurrentIndexChanged);
    gridLayout->addWidget(comboBox, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = comboBox;
}

void MainWindow::AddPictogramProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);

    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setProperty("name", name);
    lineEdit->setProperty("group", static_cast<int>(group));
    lineEdit->setProperty("type", type);
    //lineEdit->installEventFilter(focusFilter_);
    connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::on_EditingFinished);
    
    QString buttonName = QString("%1_BUTTON").arg(name);
    QToolButton* toolButtonSelectIcon = new QToolButton;
    toolButtonSelectIcon->setFixedSize(24, 24);
    toolButtonSelectIcon->setIconSize(QSize(24, 24));
    toolButtonSelectIcon->setIcon(QIcon(":/images/no-image.png"));
    toolButtonSelectIcon->setProperty("type", type);
    toolButtonSelectIcon->setProperty("group", static_cast<int>(group));
    toolButtonSelectIcon->setProperty("name", name);
    toolButtonSelectIcon->setProperty("action", "add");
    toolButtonSelectIcon->setToolTip(QString::fromLocal8Bit("�������� ������"));
    connect(toolButtonSelectIcon, &QToolButton::clicked, this, &MainWindow::on_PictogramClicked);

    QHBoxLayout* hBoxLayoutPropertyListButtons = new QHBoxLayout;
    hBoxLayoutPropertyListButtons->setContentsMargins(0, 0, 0, 0);
    hBoxLayoutPropertyListButtons->addWidget(lineEdit, 1);
    hBoxLayoutPropertyListButtons->addWidget(toolButtonSelectIcon);

    //hBoxLayoutPropertyListButtons->addStretch();
    
    QFrame* widgetPropertyListButtons = new QFrame;
    widgetPropertyListButtons->setLayout(hBoxLayoutPropertyListButtons);
    widgetPropertyListButtons->setFrameShape(QFrame::NoFrame);
    
    gridLayout->addWidget(widgetPropertyListButtons, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = lineEdit;
    tc[buttonName] = toolButtonSelectIcon;
}

void MainWindow::AddGroupWidget(QWidget* groupWidget, QString name, QString type, ControlsGroup group)
{
    auto& tc = GetControls(type, group);
    tc[name] = groupWidget;
}

bool MainWindow::ReadCurrentParameter(QString type, parameters::parameter_info& pi)
{
    TabControls& tc = GetTabControls(type);

    pi.name = qobject_cast<QLineEdit*>(tc.Properties["NAME"])->text().toStdString();
    pi.type = qobject_cast<QComboBox*>(tc.Properties["TYPE"])->currentText().toStdString();
    pi.display_name = qobject_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->text().toStdString();
    pi.description = qobject_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->toPlainText().toStdString();
    pi.required = qobject_cast<QCheckBox*>(tc.Properties["REQUIRED"])->isChecked() ? "true" : "false";
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

bool MainWindow::ReadCurrentMainInfo(parameters::info_info& mi)
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

bool MainWindow::ReadCurrentTypeInfo(QString type, parameters::type_info& ti)
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
        ti.values.push_back(std::make_pair(sl[0].toStdString(), sl[1].toStdString()));
    }

    ti.includes.clear();
    QListWidget* listWidgetIncludes = qobject_cast<QListWidget*>(tc.Info["INCLUDES"]);
    for (int i = 0; i < listWidgetIncludes->count(); ++i)
        ti.includes.push_back(listWidgetIncludes->item(i)->text().toStdString());

    return true;
}

bool MainWindow::ReadCurrentFileInfo()
{
    parameters::info_info iim{};
    if (!ReadCurrentMainInfo(iim))
        return false;
    fileInfo_.info = iim;

    if (HaveCurrentParameter("Main"))
    {
        parameters::parameter_info pim{};
        if (!ReadCurrentParameter("Main", pim))
            return false;

        if (!parameters::helper::type::set_parameter(fileInfo_, "Main", pim))
            return false;
    }

    for (const auto& type : parameters::helper::file::get_user_types(fileInfo_))
    {
        parameters::type_info tit{};
        if (!ReadCurrentTypeInfo(QString::fromStdString(type), tit))
            return false;

        if (!parameters::helper::file::set_type(fileInfo_, tit.name, tit, true))
            return false;

        if (HaveCurrentParameter(QString::fromStdString(type)))
        {
            parameters::parameter_info pit;
            if (!ReadCurrentParameter(QString::fromStdString(tit.name), pit))
                return false;

            if (!parameters::helper::type::set_parameter(fileInfo_, tit.name, pit))
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
    for (const auto& s : parameters::helper::file::get_parameter_types(fileInfo_))
        comboBoxMain->addItem(QString::fromStdString(s));
    comboBoxMain->setCurrentText(textMain);

    for (int i = 0; i < tabs_.size(); i++)
    {
        TabControls& tct = tabs_[i];

        QComboBox* comboBoxPropertyType = qobject_cast<QComboBox*>(tct.Properties["TYPE"]);
        QString textPropertyType = comboBoxPropertyType->currentText();
        comboBoxPropertyType->clear();
        for (const auto& s : parameters::helper::file::get_parameter_types(fileInfo_))
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
    for (const auto& s : parameters::helper::file::get_parameter_types(fileInfo_))
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
        for (const auto& s : parameters::helper::file::get_parameter_types(fileInfo_))
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

QWidget* MainWindow::CreateListControlWidget(int buttonSize, QString type, ControlsGroup group, QString name, QString toolTipBase, bool addDuplicate)
{
    QHBoxLayout* hBoxLayoutPropertyListButtons = new QHBoxLayout;
    hBoxLayoutPropertyListButtons->setMargin(0);

    auto& tc = GetControls(type, group);

    QString nameAdd = name + "_ADD";
    QToolButton* toolButtonPropertyListAdd = new QToolButton;
    toolButtonPropertyListAdd->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListAdd->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListAdd->setIcon(QIcon(":/images/plus.png"));
    toolButtonPropertyListAdd->setProperty("type", type);
    toolButtonPropertyListAdd->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListAdd->setProperty("name", name);
    toolButtonPropertyListAdd->setProperty("action", "add");
    toolButtonPropertyListAdd->setToolTip(QString::fromLocal8Bit("�������� %1").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListAdd);
    connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);
    tc[nameAdd] = toolButtonPropertyListAdd;

    QString nameRemove = name + "_REMOVE";
    QToolButton* toolButtonPropertyListRemove = new QToolButton;
    toolButtonPropertyListRemove->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListRemove->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListRemove->setIcon(QIcon(":/images/minus.png"));
    toolButtonPropertyListRemove->setProperty("type", type);
    toolButtonPropertyListRemove->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListRemove->setProperty("name", name);
    toolButtonPropertyListRemove->setProperty("action", "remove");
    toolButtonPropertyListRemove->setToolTip(QString::fromLocal8Bit("������� %1").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListRemove);
    connect(toolButtonPropertyListRemove, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);
    tc[nameRemove] = toolButtonPropertyListRemove;

    QString nameUp = name + "_UP";
    QToolButton* toolButtonPropertyListUp = new QToolButton;
    toolButtonPropertyListUp->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListUp->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListUp->setIcon(QIcon(":/images/up.png"));
    toolButtonPropertyListUp->setProperty("type", type);
    toolButtonPropertyListUp->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListUp->setProperty("name", name);
    toolButtonPropertyListUp->setProperty("action", "up");
    toolButtonPropertyListUp->setToolTip(QString::fromLocal8Bit("������� %1 � ������").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListUp);
    connect(toolButtonPropertyListUp, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);
    tc[nameUp] = toolButtonPropertyListUp;

    QString nameDown = name + "_DOWN";
    QToolButton* toolButtonPropertyListDown = new QToolButton;
    toolButtonPropertyListDown->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListDown->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListDown->setIcon(QIcon(":/images/down.png"));
    toolButtonPropertyListDown->setProperty("type", type);
    toolButtonPropertyListDown->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListDown->setProperty("name", name);
    toolButtonPropertyListDown->setProperty("action", "down");
    toolButtonPropertyListDown->setToolTip(QString::fromLocal8Bit("�������� %1 � ������").arg(toolTipBase));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListDown);
    connect(toolButtonPropertyListDown, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);
    tc[nameDown] = toolButtonPropertyListDown;

    if (addDuplicate)
    {
        QString nameDuplicate = name + "_DUPLICATE";
        QToolButton* toolButtonPropertyListDuplicate = new QToolButton;
        toolButtonPropertyListDuplicate->setFixedSize(buttonSize, buttonSize);
        toolButtonPropertyListDuplicate->setIconSize(QSize(buttonSize, buttonSize));
        toolButtonPropertyListDuplicate->setIcon(QIcon(":/images/duplicate.png"));
        toolButtonPropertyListDuplicate->setProperty("type", type);
        toolButtonPropertyListDuplicate->setProperty("group", static_cast<int>(group));
        toolButtonPropertyListDuplicate->setProperty("name", name);
        toolButtonPropertyListDuplicate->setProperty("action", "duplicate");
        toolButtonPropertyListDuplicate->setToolTip(QString::fromLocal8Bit("����������� %1").arg(toolTipBase));
        hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListDuplicate);
        connect(toolButtonPropertyListDuplicate, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);
        tc[nameDuplicate] = toolButtonPropertyListDuplicate;
    }

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
        QString text = QInputDialog::getText(this, "Add property", QString::fromLocal8Bit("��� ���������:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (parameters::helper::parameter::get_parameter_info(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� � ������ %1 ��� ����������").arg(text));
            return;
        }

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PARAMETERS"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        parameters::parameter_info pi{};
        pi.name = text.toStdString();
        pi.type = "string";
        pi.required = true;
        if (!parameters::helper::type::add_parameter(fileInfo_, type.toStdString(), pi))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters::helper::type::remove_parameter(fileInfo_, type.toStdString(), propertyName.toStdString()))
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
        if (!parameters::helper::type::move_parameter(fileInfo_, type.toStdString(), propertyName.toStdString(), true))
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
        if (!parameters::helper::type::move_parameter(fileInfo_, type.toStdString(), propertyName.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Parameters && name == "PARAMETERS" && action == "duplicate")
    {
        if (!ReadCurrentFileInfo())
            return;

        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PARAMETERS"]);
        if (listWidget->currentItem() == nullptr)
            return;
        QString propertyName = listWidget->currentItem()->text();

        bool ok;
        QString text = QInputDialog::getText(this, "Add property", QString::fromLocal8Bit("��� ���������:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (parameters::helper::parameter::get_parameter_info(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� � ������ %1 ��� ����������").arg(text));
            return;
        }

        // Get parameter for copy
        auto oldPi = parameters::helper::parameter::get_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString());
        if (oldPi == nullptr)
            return;

        // Add to control
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        parameters::parameter_info pi{ *oldPi };
        pi.name = text.toStdString();
        if (!parameters::helper::type::add_parameter(fileInfo_, type.toStdString(), pi))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "VALUES" && action == "add")
    {
        bool ok;
        QString textName = QInputDialog::getText(this, "Add value", QString::fromLocal8Bit("���:"), QLineEdit::Normal, "", &ok);
        if (!ok || textName.isEmpty())
            return;

        // Validate
        if (parameters::helper::type::have_value(fileInfo_, type.toStdString(), textName.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� � ������ %1 ��� ����������").arg(textName));
            return;
        }

        QString textValue = QInputDialog::getText(this, "Add value", QString::fromLocal8Bit("��������:"), QLineEdit::Normal, "", &ok);
        if (!ok || textValue.isEmpty())
            return;

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["VALUES"]);
        listWidget->addItem(new QListWidgetItem(textName + " -> " + textValue));

        // Add to fileInfo_
        if (!parameters::helper::type::add_value(fileInfo_, type.toStdString(), textName.toStdString(), textValue.toStdString()))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        const auto s = propertyName.split(" -> ");
        if (s.size() < 1)
            return;
        if (!parameters::helper::type::remove_value(fileInfo_, type.toStdString(), s[0].toStdString()))
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
        if (!parameters::helper::type::move_value(fileInfo_, type.toStdString(), s[0].toStdString(), true))
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
        if (!parameters::helper::type::move_value(fileInfo_, type.toStdString(), s[0].toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Info && name == "INCLUDES" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add include", QString::fromLocal8Bit("����:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (parameters::helper::type::have_include(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� %1 ��� ����������").arg(text));
            return;
        }

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["INCLUDES"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters::helper::type::add_include(fileInfo_, type.toStdString(), text.toStdString()))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters::helper::type::remove_include(fileInfo_, type.toStdString(), propertyName.toStdString()))
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
        if (!parameters::helper::type::move_include(fileInfo_, type.toStdString(), propertyName.toStdString(), true))
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
        if (!parameters::helper::type::move_include(fileInfo_, type.toStdString(), propertyName.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction", QString::fromLocal8Bit("��������:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters::helper::restrictions::have_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� %1 ��� ����������").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters::helper::restrictions::add_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters::helper::restrictions::remove_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
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
        if (!parameters::helper::restrictions::move_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
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
        if (!parameters::helper::restrictions::move_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction", QString::fromLocal8Bit("��������:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters::helper::restrictions::have_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� %1 ��� ����������").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters::helper::restrictions::add_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters::helper::restrictions::remove_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
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
        if (!parameters::helper::restrictions::move_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
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
        if (!parameters::helper::restrictions::move_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "SET_COUNT" && action == "add")
    {
        bool ok;
        int value = QInputDialog::getInt(this, "Add restriction", QString::fromLocal8Bit("��������:"), 0, 0, 100000, 1, &ok);
        if (!ok) return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters::helper::restrictions::have_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), QString("%1").arg(value).toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� %1 ��� ����������").arg(value));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET_COUNT"]);
        listWidget->addItem(new QListWidgetItem(QString("%1").arg(value)));

        // Add to fileInfo_
        if (!parameters::helper::restrictions::add_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), QString("%1").arg(value).toStdString()))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters::helper::restrictions::remove_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
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
        if (!parameters::helper::restrictions::move_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
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
        if (!parameters::helper::restrictions::move_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == ControlsGroup::Properties && name == "IDS" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction", QString::fromLocal8Bit("��������:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Get property name
        auto& tc = GetControls(type, group);
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Validate
        if (parameters::helper::restrictions::have_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� %1 ��� ����������").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["IDS"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!parameters::helper::restrictions::add_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
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
            QString::fromLocal8Bit("�� ������������� ������ ������� �������� %1?").arg(value), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Get property name
        QString propertyName = qobject_cast<QLineEdit*>(tc["NAME"])->text();

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (!parameters::helper::restrictions::remove_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
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
        if (!parameters::helper::restrictions::move_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
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
        if (!parameters::helper::restrictions::move_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
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

    parameters::parameter_info pic{};
    if (current != nullptr)
    {
        parameters::parameter_info* ppic = parameters::helper::parameter::get_parameter_info(fileInfo_, type.toStdString(), current->text().toStdString());
        if (ppic != nullptr) pic = *ppic;
    }

    if (previous != nullptr)
    {
        parameters::parameter_info pip{};
        if (!ReadCurrentParameter(type, pip))
            return;

        if (!parameters::helper::type::set_parameter(fileInfo_, type.toStdString(), pip))
            return;
    }

    TabControls& tc = GetTabControls(type);

    qobject_cast<QLineEdit*>(tc.Properties["NAME"])->setText(QString::fromStdString(pic.name));
    qobject_cast<QComboBox*>(tc.Properties["TYPE"])->setCurrentText(QString::fromStdString(pic.type));
    qobject_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->setText(QString::fromStdString(pic.display_name));
    qobject_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->setPlainText(QString::fromStdString(pic.description));
    qobject_cast<QCheckBox*>(tc.Properties["REQUIRED"])->setChecked(pic.required == "true");
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
    else if (group == MainWindow::ControlsGroup::Info && name == "PICTOGRAM")
    {
        // Main PICTOGRAM
        UpdatePictogram();
    }
    else if (group == MainWindow::ControlsGroup::Info && name == "NAME")
    {
        // Type NAME
        QString oldName = type;
        QString newName = lineEdit->text();

        if (parameters::helper::type::get_type_info(fileInfo_, newName.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("��� � ������ %1 ��� ����������").arg(newName));
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
        if (!parameters::helper::file::rename_type(fileInfo_, oldName.toStdString(), newName.toStdString()))
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
                QMessageBox::critical(this, "Error", QString::fromLocal8Bit("�������� � ������ %1 ��� ����������").arg(newName));
                if (listWidget->selectedItems().size() > 0)
                    lineEdit->setText(oldName);
                return;
            }
        }
        listWidget->selectedItems()[0]->setText(newName);

        // Update fileInfo_
        if (!parameters::helper::type::rename_parameter(fileInfo_, type.toStdString(), oldName.toStdString(), newName.toStdString()))
            return;
    }

    modified_ = true;
    UpdateWindowTitle();
}

void MainWindow::on_CurrentIndexChanged(int index)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
    QString name = comboBox->property("name").toString();
    MainWindow::ControlsGroup group = static_cast<MainWindow::ControlsGroup>(comboBox->property("group").toInt());
    QString type = comboBox->property("type").toString();

    if (group == MainWindow::ControlsGroup::Info && name == "TYPE")
    {
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
                QString message = QString::fromLocal8Bit("��� %1 ������������ ��� ��������� � ������ ����,\n�� ��� ����� yml ����������� ������������� ������ � �������� array<%1>. ����:\n").arg(type);
                for (const auto& s : usedInTypes)
                    message += s + "\n";
                QMessageBox::StandardButton resBtn = QMessageBox::critical(this, "parameters_composer", message);

                comboBox->setCurrentText("enum"); // !!! need get value from fileInfo_
                return;
            }
        }

        parameters::type_info tit{};
        if (!ReadCurrentTypeInfo(type, tit))
            return;

        auto ti = parameters::helper::type::get_type_info(fileInfo_, type.toStdString());
        ti->type = tit.type;
        //if (!parameters::helper::set_type_info(fileInfo_, type.toStdString(), tit, true))
        //    return;

        FillPropertyTypeNames();

        modified_ = true;
        UpdateWindowTitle();
    }
    else if (group == MainWindow::ControlsGroup::Properties && name == "TYPE")
    {
        modified_ = true;
        UpdateWindowTitle();
    }
}

void MainWindow::Update()
{
    UpdateMain();
    for (const auto& type : parameters::helper::file::get_user_types(fileInfo_))
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

    UpdatePictogram();

    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.Parameters["PARAMETERS"]);
    for (const auto& pi : fileInfo_.parameters)
        listWidget->addItem(QString::fromStdString(pi.name));
    if (listWidget->count() > 0)
        listWidget->setCurrentRow(0);
}

void MainWindow::UpdateType(QString type)
{
    TabControls& tc = GetTabControls(type);
    const auto ti = parameters::helper::type::get_type_info(fileInfo_, type.toStdString());

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

void MainWindow::UpdatePictogram()
{
    auto& tc = GetControls("Main", MainWindow::ControlsGroup::Info);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(tc["PICTOGRAM"]);
    QToolButton* toolButton = qobject_cast<QToolButton*>(tc["PICTOGRAM_BUTTON"]);

    std::string ds = base64_decode(lineEdit->text().toStdString());
    QByteArray ba(ds.c_str(), static_cast<int>(ds.size()));
    QPixmap px;
    bool loaded = false;
    try
    {
        loaded = px.loadFromData(ba);
    }
    catch (...)
    {
        loaded = false;
    }
    if (!loaded)
        toolButton->setIcon(QIcon(":/images/no-image.png"));
    else
        toolButton->setIcon(QIcon(px));
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

    parameters::file_info fi = fileInfo_;

    std::string message;
    if (!parameters::helper::file::validate(fi, message))
    {
        QMessageBox::critical(this, "Validate error", QString::fromLocal8Bit(message.c_str()));
        return false;
    }

    bool have_type_loop = false;
    if (!parameters::helper::file::rearrange_types(fi, have_type_loop))
    {
        QMessageBox::critical(this, "Rearrange error", QString::fromLocal8Bit("������ ������������������ ���������������� ����� ����� �����������"));
        return false;
    }
    else if (have_type_loop)
    {
        QMessageBox::warning(this, "Rearrange", QString::fromLocal8Bit("���������� ����������� ����������� � �����.\n���� ����� ��������, �� ��� ���������� ������ ������� �����������!"));
    }

    if (is_json)
    {
        if (!parameters::json::writer::write(fileName.toStdString(), fi))
            return false;
    }
    else
    {
        parameters::yaml::writer writer{};
        if (!writer.write(fileName.toStdString(), fi))
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

    parameters::struct_types st{ parameters::struct_types::file_info };
    if (type == "Main")
    {
        if (group == ControlsGroup::Info)
            st = parameters::struct_types::info_info;
        else if (group == ControlsGroup::Properties)
            st = parameters::struct_types::parameter_info;
        else if (group == ControlsGroup::Parameters)
            st = parameters::struct_types::file_info;
    }
    else
    {
        if (group == ControlsGroup::Info)
            st = parameters::struct_types::type_info;
        else if (group == ControlsGroup::Properties)
            st = parameters::struct_types::parameter_info;
        else if (group == ControlsGroup::Parameters)
            st = parameters::struct_types::type_info;
    }

    QString text = QString::fromLocal8Bit(parameters::helper::common::get_hint_html_as_cp1251(st, name.toStdString()).c_str());

    // restrictions_info is a part of parameter_info, but on gui we not divide it
    if (text == "" && st == parameters::struct_types::parameter_info)
        text = QString::fromLocal8Bit(parameters::helper::common::get_hint_html_as_cp1251(parameters::struct_types::restrictions_info, name.toStdString()).c_str());

    plainTextEditHint_->clear();
    plainTextEditHint_->appendHtml(QString("<p style='font-weight: bold; font-size: 14px;'>%1</p>").arg(name));
    plainTextEditHint_->appendHtml(text);
}

void MainWindow::on_PictogramClicked()
{
    QFileDialog dialog(this);
    //dialog.setNameFilters({ "PNG Files (*.png)" });
    dialog.setNameFilters({ "PNG Files (*.png)", "ICO (*.ico)" });
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() == 0)
        return;

    //bool is_png = (dialog.selectedNameFilter() == "PNG Files (*.png)");
    std::ifstream input(fileNames[0].toStdString(), std::ios::binary);
    if (!input.is_open())
        return;
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

    std::string s = base64_encode(buffer.data(), buffer.size());
    auto& tc = GetControls("Main", MainWindow::ControlsGroup::Info);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(tc["PICTOGRAM"]);
    lineEdit->setText(QString::fromStdString(s));

    UpdatePictogram();

    modified_ = true;
    UpdateWindowTitle();
}
