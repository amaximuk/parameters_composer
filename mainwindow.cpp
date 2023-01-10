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

#include "yaml_parser.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    currentFileName_ = "Untitled";
    fileInfo_ = {};

    CreateUi();

    setWindowIcon(QIcon(":/images/parameters.png"));
    setWindowTitle("parameters_composer - " + currentFileName_);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        "Are you sure want to exit?", QMessageBox::No | QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
        event->accept();
    else
        event->ignore();
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
        MainWindow::TabControls tc{};
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

QMap<QString, QObject*>& MainWindow::GetControls(QString type, MainWindow::ControlsGroup group)
{
    MainWindow::TabControls& tc = GetTabControls(type);
    switch (group)
    {
    case MainWindow::ControlsGroup::Info:
        return tc.Info;
    case MainWindow::ControlsGroup::PropertyList:
        return tc.PropertyList;
    case MainWindow::ControlsGroup::Properties:
        return tc.Properties;
    default:
        return tc.Info; // fake
    }
}

void MainWindow::on_NewFile_action()
{
}

void MainWindow::on_Quit_action()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        "Are you sure want to exit?", QMessageBox::No | QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
        QApplication::quit();
}

void MainWindow::on_OpenFile_action()
{
    QFileDialog dialog(this);
    dialog.setNameFilter("Parameters Compiler Files (*.yml *.yaml *.json)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() > 0)
    {
        qDebug() << fileNames[0];
        yaml::yaml_parser parser(false);
        if (parser.parse(fileNames[0].toStdString()))
        {
            currentFileName_ = fileNames[0];
            setWindowTitle("parameters_composer - " + currentFileName_);
            fileInfo_ = parser.get_file_info();

            {
                TabControls& tc = tabs_[0];

                qobject_cast<QLineEdit*>(tc.Info["ID"])->setText(QString::fromStdString(fileInfo_.info.yml.id));
                qobject_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->setText(QString::fromStdString(fileInfo_.info.yml.display_name));
                qobject_cast<QLineEdit*>(tc.Info["CATEGORY"])->setText(QString::fromStdString(fileInfo_.info.yml.category));
                qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString::fromStdString(fileInfo_.info.yml.description));
                qobject_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->setText(QString::fromStdString(fileInfo_.info.yml.pictogram));
                qobject_cast<QLineEdit*>(tc.Info["HINT"])->setText(QString::fromStdString(fileInfo_.info.yml.hint));
                qobject_cast<QLineEdit*>(tc.Info["AUTHOR"])->setText(QString::fromStdString(fileInfo_.info.yml.author));
                qobject_cast<QLineEdit*>(tc.Info["WIKI"])->setText(QString::fromStdString(fileInfo_.info.yml.wiki));
                qobject_cast<QLineEdit*>(tc.Info["WIKI"])->setCursorPosition(0);

                QListWidget* listWidget = dynamic_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
                listWidget->clear();
                for (const auto& pi : fileInfo_.parameters)
                    listWidget->addItem(QString(pi.yml.name.c_str()));
                if (listWidget->count() > 0)
                    listWidget->setCurrentRow(0);
            }

            for (const auto& ti : fileInfo_.types)
            {
                QString type(ti.yml.name.c_str());

                QWidget* widgetTabType = CreateTypeTabWidget(type);
                dynamic_cast<QTabWidget*>(centralWidget())->addTab(widgetTabType, type);
                
                TabControls& tc = GetTabControls(type);

                dynamic_cast<QLineEdit*>(tc.Info["NAME"])->setText(QString(ti.yml.name.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["TYPE"])->setText(QString(ti.yml.type.c_str()));
                dynamic_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString(ti.yml.description.c_str()));

                QListWidget* listWidgetValues = dynamic_cast<QListWidget*>(tc.Info["VALUES"]);
                listWidgetValues->clear();
                for (const auto& v : ti.yml.values)
                    listWidgetValues->addItem(QString(v.first.c_str()) + " -> " + QString(v.second.c_str()));

                QListWidget* listWidgetIncludes = dynamic_cast<QListWidget*>(tc.Info["INCLUDES"]);
                listWidgetIncludes->clear();
                for (const auto& v : ti.yml.includes)
                    listWidgetIncludes->addItem(QString(v.c_str()));

                QListWidget* listWidget = dynamic_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
                listWidget->clear();
                for (const auto& pi : ti.parameters)
                    listWidget->addItem(QString(pi.yml.name.c_str()));
                if (listWidget->count() > 0)
                    listWidget->setCurrentRow(0);
            }
        }
        else
        {
            QMessageBox::critical(this, "Error", "File parsing error: " + fileNames[0]);
        }
    }
}
void MainWindow::on_SaveFile_action()
{

}

void MainWindow::on_SaveAsFile_action()
{
    QFileDialog dialog(this);
    dialog.setNameFilter("Parameters Compiler Files (*.yml)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("yml");

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() > 0)
    {
        qDebug() << fileNames[0];

        if (!SaveCurrent())
            return;

        if (!Validate())
            return;

        if (!RearrangeTypes())
            return;

        YAML::Emitter emitter;
        if (!WriteCurrent(emitter))
            return;

        QFile file(fileNames[0]);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(emitter.c_str());
            //QTextStream stream(&file);
            //std::string s(emitter.c_str());
            //stream << QString::fromUtf8(s.c_str());
        }
    }
}

void MainWindow::CreateMenu()
{
    QAction* newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::on_NewFile_action);

    QAction* openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::on_OpenFile_action);

    QAction* saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save file"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::on_SaveFile_action);

    QAction* saveAsAct = new QAction(tr("&Save as"), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save file as"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::on_SaveAsFile_action);

    QAction* quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit application"));
    connect(quitAct, &QAction::triggered, this, &MainWindow::on_Quit_action);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);
}

void MainWindow::CreateUi()
{
    resize(1000, 600);

    CreateMenu();

    QTabWidget* tabWidget = new QTabWidget;
    QWidget* widgetTabProperties = CreateMainTabWidget();
    tabWidget->addTab(widgetTabProperties, "Main");

    setCentralWidget(tabWidget);
}

QWidget* MainWindow::CreateMainTabWidget()
{
    QWidget* widgetTabProperties = new QWidget;

    QWidget* widgetSplitterInfo = CreateMainTabInfoWidget();
    QWidget* widgetSplitterPropertyList = CreatePropertyListWidget("Main");
    QWidget* widgetSplitterProperties = CreatePropertiesWidget("Main");

    QSplitter* tabHSplitter = new QSplitter(Qt::Horizontal);
    tabHSplitter->addWidget(widgetSplitterInfo);
    tabHSplitter->addWidget(widgetSplitterPropertyList);
    tabHSplitter->addWidget(widgetSplitterProperties);
    tabHSplitter->setStretchFactor(0, 1);
    tabHSplitter->setStretchFactor(1, 0);
    tabHSplitter->setStretchFactor(2, 1);

    QVBoxLayout* vBoxLayoutSplitter = new QVBoxLayout;
    vBoxLayoutSplitter->addWidget(tabHSplitter);
    widgetTabProperties->setLayout(vBoxLayoutSplitter);

    return widgetTabProperties;
}

QWidget* MainWindow::CreateMainTabInfoWidget()
{
    QGridLayout* gridLayoutInfo = new QGridLayout;
 
    TabControls& tc = GetTabControls("Main");

    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "INFO", "font-weight: bold; font-size: 14px", index++);
    AddLineEditRequiredProperty(gridLayoutInfo, "ID", index++, "Main", MainWindow::ControlsGroup::Info);
    //AddLineEditProperty(gridLayoutInfo, "ID", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "DISPLAY_NAME", index++, tc.Info);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "CATEGORY", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "PICTOGRAM", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "HINT", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "AUTHOR", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "WIKI", index++, tc.Info);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

QWidget* MainWindow::CreatePropertyListWidget(QString type)
{
    QLabel* labelPropertyListHeader = new QLabel;
    labelPropertyListHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelPropertyListHeader->setText("PROPERITES");

    QWidget* widgetPropertyListButtons = CreateListControlWidget(32, "PropertiesTab", "PropertyList", type);

    QListWidget* listWidget = new QListWidget;
    listWidget->setProperty("type", type);
    connect(listWidget, &QListWidget::currentItemChanged, this, &MainWindow::on_listWidgetProperties_currentItemChanged);

    QVBoxLayout* vBoxLayoutPropertyList = new QVBoxLayout;
    vBoxLayoutPropertyList->addWidget(labelPropertyListHeader, 0, Qt::AlignCenter);
    vBoxLayoutPropertyList->addWidget(widgetPropertyListButtons);
    vBoxLayoutPropertyList->addWidget(listWidget, 1);
    vBoxLayoutPropertyList->addStretch();

    QWidget* widgetSplitterPropertyList = new QWidget;
    widgetSplitterPropertyList->setLayout(vBoxLayoutPropertyList);

    TabControls& tc = GetTabControls(type);
    tc.PropertyList["PROPERTIES"] = listWidget;

    return widgetSplitterPropertyList;
}

void MainWindow::AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QLineEdit* lineEdit = new QLineEdit;
    gridLayout->addWidget(lineEdit, index, 1);
    mapControls[name] = lineEdit;
}

void MainWindow::AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QPlainTextEdit* plainTextEdit = new QPlainTextEdit;
    gridLayout->addWidget(plainTextEdit, index, 1);
    mapControls[name] = plainTextEdit;
}

void MainWindow::AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QCheckBox* checkBox = new QCheckBox;
    gridLayout->addWidget(checkBox, index, 1);
    mapControls[name] = checkBox;
}

void MainWindow::AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index)
{
    QLabel* label = new QLabel;
    label->setStyleSheet(style);
    label->setText(text);
    gridLayout->addWidget(label, index, 0, 1, 2, Qt::AlignCenter);
}

//QComboBox

void MainWindow::AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId, QMap<QString, QObject*>& mapControls, QString type)
{
    QWidget* widgetPropertiesRestrictionsSetButtons = CreateListControlWidget(24, tabId, listControlId, type);
    QListWidget* listWidget = new QListWidget;
    //listWidget->setProperty("type", type);
    QVBoxLayout* vBoxLayoutPropertiesRestrictionsSet = new QVBoxLayout;
    vBoxLayoutPropertiesRestrictionsSet->addWidget(widgetPropertiesRestrictionsSetButtons);
    vBoxLayoutPropertiesRestrictionsSet->addWidget(listWidget, 1);
    vBoxLayoutPropertiesRestrictionsSet->addStretch();
    vBoxLayoutPropertiesRestrictionsSet->setMargin(0);
    QWidget* widgetPropertiesRestrictionsSet = new QWidget;
    widgetPropertiesRestrictionsSet->setLayout(vBoxLayoutPropertiesRestrictionsSet);
    gridLayout->addWidget(new QLabel(name), index, 0);
    gridLayout->addWidget(widgetPropertiesRestrictionsSet, index, 1);
    mapControls[name] = listWidget;
}

void MainWindow::AddLineEditRequiredProperty(QGridLayout* gridLayout, QString name, int index, QString type, MainWindow::ControlsGroup group)
{
    QLabel* label = new QLabel;
    label->setStyleSheet("font-weight: bold");
    label->setText(name);

    gridLayout->addWidget(label, index, 0);
    QLineEdit* lineEdit = new QLineEdit;
    lineEdit->setProperty("name", name);
    lineEdit->setProperty("group", static_cast<int>(group));
    lineEdit->setProperty("type", type);
    connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::on_editingFinished);
    gridLayout->addWidget(lineEdit, index, 1);

    QMap<QString, QObject*>& cnt = GetControls(type, group);
    cnt[name] = lineEdit;
}

bool MainWindow::ReadCurrentParameters(QString type, yaml::parameter_info& pi)
{
    TabControls& tc = GetTabControls(type);

    pi.yml.name = dynamic_cast<QLineEdit*>(tc.Properties["NAME"])->text().toStdString();
    pi.yml.type = dynamic_cast<QLineEdit*>(tc.Properties["TYPE"])->text().toStdString();
    pi.yml.display_name = dynamic_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->text().toStdString();
    pi.yml.description = dynamic_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->toPlainText().toStdString();
    pi.yml.required = dynamic_cast<QCheckBox*>(tc.Properties["REQUIRED"])->isChecked();
    pi.yml.default_ = dynamic_cast<QLineEdit*>(tc.Properties["DEFAULT"])->text().toStdString();
    pi.yml.hint = dynamic_cast<QLineEdit*>(tc.Properties["HINT"])->text().toStdString();

    pi.yml.restrictions.min = dynamic_cast<QLineEdit*>(tc.Properties["MIN"])->text().toStdString();
    pi.yml.restrictions.max = dynamic_cast<QLineEdit*>(tc.Properties["MAX"])->text().toStdString();
    pi.yml.restrictions.set_.clear();
    QListWidget* listWidgetSet = dynamic_cast<QListWidget*>(tc.Properties["SET"]);
    for (int i = 0; i < listWidgetSet->count(); ++i)
        pi.yml.restrictions.set_.push_back(listWidgetSet->item(i)->text().toStdString());

    pi.yml.restrictions.min_count = dynamic_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->text().toStdString();
    pi.yml.restrictions.max_count = dynamic_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->text().toStdString();
    pi.yml.restrictions.set_count.clear();
    QListWidget* listWidgetSetCount = dynamic_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    for (int i = 0; i < listWidgetSetCount->count(); ++i)
        pi.yml.restrictions.set_count.push_back(listWidgetSetCount->item(i)->text().toStdString());

    pi.yml.restrictions.category = dynamic_cast<QLineEdit*>(tc.Properties["CATEGORY"])->text().toStdString();

    pi.yml.restrictions.ids.clear();
    QListWidget* listWidgetIds = dynamic_cast<QListWidget*>(tc.Properties["IDS"]);
    for (int i = 0; i < listWidgetIds->count(); ++i)
        pi.yml.restrictions.ids.push_back(listWidgetIds->item(i)->text().toStdString());

    pi.yml.restrictions.max_length = dynamic_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->text().toStdString();

    return true;
}

bool MainWindow::ReadCurrentMainInfo(QString type, yaml::info_info& mi)
{
    TabControls& tc = GetTabControls("Main");

    mi.yml.id = dynamic_cast<QLineEdit*>(tc.Info["ID"])->text().toStdString();
    mi.yml.display_name = dynamic_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->text().toStdString();
    mi.yml.description = dynamic_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->toPlainText().toStdString();
    mi.yml.category = dynamic_cast<QLineEdit*>(tc.Info["CATEGORY"])->text().toStdString();
    mi.yml.pictogram = dynamic_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->text().toStdString();
    mi.yml.hint = dynamic_cast<QLineEdit*>(tc.Info["HINT"])->text().toStdString();
    mi.yml.author = dynamic_cast<QLineEdit*>(tc.Info["AUTHOR"])->text().toStdString();
    mi.yml.wiki = dynamic_cast<QLineEdit*>(tc.Info["WIKI"])->text().toStdString();

    return true;
}

bool MainWindow::ReadCurrentTypeInfo(QString type, yaml::type_info& ti)
{
    TabControls& tc = GetTabControls(type);

    ti.yml.name = dynamic_cast<QLineEdit*>(tc.Info["NAME"])->text().toStdString();
    ti.yml.type = dynamic_cast<QLineEdit*>(tc.Info["TYPE"])->text().toStdString();
    ti.yml.description = dynamic_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->toPlainText().toStdString();

    ti.yml.values.clear();
    QListWidget* listWidgetValues = dynamic_cast<QListWidget*>(tc.Info["VALUES"]);
    for (int i = 0; i < listWidgetValues->count(); ++i)
    {
        QString v = listWidgetValues->item(i)->text();
        QStringList sl = v.split(" -> ");
        ti.yml.values.push_back({ sl[0].toStdString(), sl[1].toStdString() });
    }

    ti.yml.includes.clear();
    QListWidget* listWidgetIncludes = dynamic_cast<QListWidget*>(tc.Info["INCLUDES"]);
    for (int i = 0; i < listWidgetIncludes->count(); ++i)
        ti.yml.includes.push_back(listWidgetIncludes->item(i)->text().toStdString());

    return true;
}

bool MainWindow::SaveCurrentParameters(QString type)
{
    yaml::parameter_info pi;
    if (!ReadCurrentParameters(type, pi))
        return false;

    if (type == "Main")
    {
        for (auto& p : fileInfo_.parameters)
        {
            if (p.yml.name == pi.yml.name)
            {
                p = pi;
                break;
            }
        }
    }
    else
    {
        for (auto& t : fileInfo_.types)
        {
            if (QString(t.yml.name.c_str()) == type)
            {
                for (auto& p : t.parameters)
                {
                    if (p.yml.name == pi.yml.name)
                    {
                        p = pi;
                        break;
                    }
                }
                break;
            }
        }
    }

    return true;
}

bool MainWindow::SaveCurrentInfo(QString type)
{
    if (type == "Main")
    {
        yaml::info_info mi;
        if (!ReadCurrentMainInfo(type, mi))
            return false;
        fileInfo_.info = mi;
    }
    else
    {
        for (auto& t : fileInfo_.types)
        {
            if (QString(t.yml.name.c_str()) == type)
            {
                yaml::type_info ti;
                if (!ReadCurrentTypeInfo(type, ti))
                    return false;

                yaml::type_info ti_ = t;
                t = ti;
                t.parameters = ti_.parameters; // need optimize !!!
                break;
            }
        }
    }

    return true;
}

bool MainWindow::SaveCurrent()
{
    for (const auto& t : tabs_)
    {
        if (!SaveCurrentInfo(t.Name))
            return false;

        if (!SaveCurrentParameters(t.Name))
            return false;
    }

    return true;
}

bool MainWindow::Validate()
{
    if (fileInfo_.info.yml.id == "")
    {
        QMessageBox::critical(this, "Error", "Main ID required");
        return false;
    }

    for (const auto& p : fileInfo_.parameters)
    {
        if (p.yml.name == "")
        {
            QMessageBox::critical(this, "Error", "Main parameter NAME required");
            return false;
        }

        if (p.yml.type == "") // add list of values !!!
        {
            QMessageBox::critical(this, "Error", "Main parameter TYPE required");
            return false;
        }
    }

    for (const auto& t : fileInfo_.types)
    {
        if (t.yml.name == "")
        {
            QMessageBox::critical(this, "Error", "Type NAME required");
            return false;
        }

        for (const auto& tv : t.parameters)
        {
            if (tv.yml.name == "")
            {
                QMessageBox::critical(this, "Error", "Main parameter NAME required");
                return false;
            }

            if (tv.yml.type == "") // add list of values !!!
            {
                QMessageBox::critical(this, "Error", "Main parameter TYPE required");
                return false;
            }
        }
    }

    return true;
}

bool MainWindow::Contains(QList<ll*>& list, QString value)
{
    for (const auto v : list)
    {
        if (v->Name == value)
        {
            return true;
        }
    }
    return false;
}

void MainWindow::FlatList(ll* type)
{
    for (auto llt : type->List)
    {
        FlatList(llt);
        for (int i = 0; i < llt->FlatList.size(); i++)
        {
            if (type->FlatList.size() <= i)
            {
                type->FlatList.push_back(llt->FlatList[i]);
            }
            else
            {
                //for (int j = 0; j < llt->FlatList[i]->List.size(); j++)
                //{
                //    if (!Contains(type->FlatList[i]->List, llt->FlatList[i]->List[j]->Name))
                //        type->FlatList[i]->List.push_back(llt->FlatList[i]->List[j]);
                //}
                for (const auto v : llt->FlatList[i]->List)
                {
                    if (!Contains(type->FlatList[i]->List, v->Name))
                        type->FlatList[i]->List.push_back(v);
                }
            }
        }
    }
    type->FlatList.insert(0, new ll(type->Name));
    type->FlatList[0]->List.push_back(new ll(type->Name));
}

void MainWindow::AddTypes(ll* type, int level)
{
    yaml::yaml_parser yp(false);

    for (const auto& ti : fileInfo_.types)
    {
        if (QString(ti.yml.name.c_str()) == type->Name)
        {
            for (const auto& pi : ti.parameters)
            {
                QString ts = QString(pi.yml.type.c_str());
                if (ts.startsWith("array") && ts.length() > 7)
                    ts = ts.mid(6, ts.length() - 7);


                // Validate, search loops
                if (ts == type->Name)
                {
                    QMessageBox::warning(this, "Warning", "Type loop found");
                    continue;
                }

                MainWindow::ll* parent = type->Parent;
                while (parent != nullptr)
                {
                    if (parent->Name == type->Name)
                    {
                        QMessageBox::warning(this, "Warning", "Type loop found");
                        continue;
                    }
                    parent = parent->Parent;
                }


                ll* llt = new ll(ts, type);
                if (!yp.is_inner_type(ts.toStdString()) && !Contains(type->List, ts))
                {
                    if (level < 10)
                        AddTypes(llt, ++level);
                    type->List.push_back(llt);

                    //for (size_t i = 0; i < llt.FlatList.size(); i++)
                    //{
                    //    if (type.FlatList.size() <= i)
                    //    {
                    //        type.FlatList.push_back(llt.FlatList[i]);
                    //    }
                    //    else
                    //    {
                    //        for (const auto& v : llt.FlatList[i].List)
                    //        {
                    //            if (!type.FlatList[i].List.contains(v))
                    //                type.FlatList[i].List.push_back(v);
                    //        }
                    //    }
                    //}
                    //type.FlatList.insert(0, ll(ts));
                }
            }
            break;
        }
    }
}

bool MainWindow::RearrangeTypes()
{
    yaml::yaml_parser yp(false);
    QList<QString> sorted_names;
    bool found_new = true;
    while (found_new)
    {
        found_new = false;
        for (const auto& ti : fileInfo_.types)
        {
            QString tn = QString(ti.yml.name.c_str());
            if (sorted_names.contains(tn))
                continue;

            bool have_unresolved = false;
            for (const auto& pi : ti.parameters)
            {
                QString ts = QString(pi.yml.type.c_str());
                if (ts.startsWith("array") && ts.length() > 7)
                    ts = ts.mid(6, ts.length() - 7);

                if (!yp.is_inner_type(ts.toStdString()) && !sorted_names.contains(ts))
                {
                    have_unresolved = true;
                    break;
                }
            }

            if (!have_unresolved)
            {
                sorted_names.push_back(tn);
                found_new = true;
            }
        }
    }

    if (fileInfo_.types.size() > sorted_names.size())
    {
        QMessageBox::warning(this, "Warning", "Type loop found");
        for (const auto& ti : fileInfo_.types)
        {
            QString tn = QString(ti.yml.name.c_str());
            if (!sorted_names.contains(tn))
                sorted_names.push_back(tn);
        }
    }
  
    std::vector<yaml::type_info> sorted_types;
    for (const auto& sn : sorted_names)
    {
        for (const auto& ti : fileInfo_.types)
        {
            if (sn == QString(ti.yml.name.c_str()))
            {
                sorted_types.push_back(ti);
            }
        }
    }

    fileInfo_.types = std::move(sorted_types);

    return true;
}

bool MainWindow::WriteCurrent(YAML::Emitter& emitter)
{
    emitter << YAML::BeginMap;
    WriteInfo(emitter, fileInfo_.info);

    if (fileInfo_.types.size() > 0)
    {
        emitter << YAML::Key << "TYPES";
        emitter << YAML::Value << YAML::BeginSeq;
        for (const auto& ti : fileInfo_.types)
            WriteType(emitter, ti);
        emitter << YAML::EndSeq;
    }

    if (fileInfo_.parameters.size() > 0)
    {
        emitter << YAML::Key << "PARAMETERS";
        emitter << YAML::Value << YAML::BeginSeq;
        for (const auto& pi : fileInfo_.parameters)
            WriteParameter(emitter, pi);
        emitter << YAML::EndSeq;
    }

    emitter << YAML::EndMap;

    return true;
}

bool MainWindow::WriteInfo(YAML::Emitter& emitter, yaml::info_info ii)
{
    emitter << YAML::Key << "INFO";
    emitter << YAML::Value << YAML::BeginMap;

    emitter << YAML::Key << "ID";
    emitter << YAML::Value << ii.yml.id;
    if (ii.yml.display_name != "")
    {
        emitter << YAML::Key << "DISPLAY_NAME";
        emitter << YAML::Value << ii.yml.display_name;
    }
    if (ii.yml.category != "")
    {
        emitter << YAML::Key << "CATEGORY";
        emitter << YAML::Value << ii.yml.category;
    }
    if (ii.yml.description != "")
    {
        emitter << YAML::Key << "DESCRIPTION";
        emitter << YAML::Value << YAML::Literal << ii.yml.description;
    }
    if (ii.yml.pictogram != "")
    {
        emitter << YAML::Key << "PICTOGRAM";
        emitter << YAML::Value << ii.yml.pictogram;
    }
    if (ii.yml.hint != "")
    {
        emitter << YAML::Key << "HINT";
        emitter << YAML::Value << ii.yml.hint;
    }
    if (ii.yml.author != "")
    {
        emitter << YAML::Key << "AUTHOR";
        emitter << YAML::Value << ii.yml.author;
    }
    if (ii.yml.wiki != "")
    {
        emitter << YAML::Key << "WIKI";
        emitter << YAML::Value << ii.yml.wiki;
    }

    emitter << YAML::EndMap;

    return true;
}

bool MainWindow::WriteType(YAML::Emitter& emitter, yaml::type_info ti)
{
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "NAME";
    emitter << YAML::Value << ti.yml.name;
    if (ti.yml.type != "" && ti.yml.type != "yml")
    {
        emitter << YAML::Key << "TYPE";
        emitter << YAML::Value << ti.yml.type;
    }
    if (fileInfo_.info.yml.description != "")
    {
        emitter << YAML::Key << "DESCRIPTION";
        emitter << YAML::Value << YAML::Literal << ti.yml.description;
    }
    if (ti.yml.values.size() > 0)
    {
        emitter << YAML::Key << "VALUES";
        emitter << YAML::Value << YAML::BeginMap;
        for (const auto& v : ti.yml.values)
        {
            emitter << YAML::Key << v.first;
            emitter << YAML::Value << v.second;
        }
        emitter << YAML::EndMap;
    }
    if (ti.yml.includes.size() > 0)
    {
        emitter << YAML::Key << "INCLUDES";
        emitter << YAML::Value << ti.yml.includes;
    }
    if (ti.parameters.size() > 0)
    {
        emitter << YAML::Key << "PARAMETERS";
        emitter << YAML::Value << YAML::BeginSeq;
        for (const auto& pi : ti.parameters)
            WriteParameter(emitter, pi);
        emitter << YAML::EndSeq;
    }
    emitter << YAML::EndMap;

    return true;
}

bool MainWindow::WriteParameter(YAML::Emitter& emitter, yaml::parameter_info pi)
{
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "NAME";
    emitter << YAML::Value << pi.yml.name;
    emitter << YAML::Key << "TYPE";
    emitter << YAML::Value << pi.yml.type;
    if (pi.yml.display_name != "")
    {
        emitter << YAML::Key << "DISPLAY_NAME";
        emitter << YAML::Value << pi.yml.display_name;
    }
    if (pi.yml.description != "")
    {
        emitter << YAML::Key << "DESCRIPTION";
        emitter << YAML::Value << YAML::Literal << pi.yml.description;
    }
    if (pi.yml.required != true)
    {
        emitter << YAML::Key << "REQUIRED";
        emitter << YAML::Value << YAML::TrueFalseBool << pi.yml.required;
    }
    if (pi.yml.default_ != "")
    {
        emitter << YAML::Key << "DEFAULT";
        emitter << YAML::Value << pi.yml.default_;
    }
    if (pi.yml.hint != "")
    {
        emitter << YAML::Key << "HINT";
        emitter << YAML::Value << pi.yml.hint;
    }

    if (pi.yml.restrictions.min != "" || pi.yml.restrictions.max != "" || pi.yml.restrictions.set_.size() > 0 ||
        pi.yml.restrictions.min_count != "" || pi.yml.restrictions.max_count != "" || pi.yml.restrictions.set_count.size() > 0 ||
        pi.yml.restrictions.category != "" || pi.yml.restrictions.ids.size() > 0 || pi.yml.restrictions.max_length != "")
    {
        emitter << YAML::Key << "RESTRICTIONS";
        emitter << YAML::Value << YAML::BeginMap;
        if (pi.yml.restrictions.min != "")
        {
            emitter << YAML::Key << "MIN";
            emitter << YAML::Value << pi.yml.restrictions.min;
        }
        if (pi.yml.restrictions.max != "")
        {
            emitter << YAML::Key << "MAX";
            emitter << YAML::Value << pi.yml.restrictions.max;
        }
        if (pi.yml.restrictions.set_.size() > 0)
        {
            emitter << YAML::Key << "SET";
            emitter << YAML::Value << YAML::Flow << pi.yml.restrictions.set_;
        }
        if (pi.yml.restrictions.min_count != "")
        {
            emitter << YAML::Key << "MIN_COUNT";
            emitter << YAML::Value << pi.yml.restrictions.min_count;
        }
        if (pi.yml.restrictions.max_count != "")
        {
            emitter << YAML::Key << "MAX_COUNT";
            emitter << YAML::Value << pi.yml.restrictions.max_count;
        }
        if (pi.yml.restrictions.set_count.size() > 0)
        {
            emitter << YAML::Key << "SET_COUNT";
            emitter << YAML::Value << YAML::Flow << pi.yml.restrictions.set_count;
        }
        if (pi.yml.restrictions.category != "")
        {
            emitter << YAML::Key << "CATEGORY";
            emitter << YAML::Value << pi.yml.restrictions.category;
        }
        if (pi.yml.restrictions.ids.size() > 0)
        {
            emitter << YAML::Key << "IDS";
            emitter << YAML::Value << YAML::Flow << pi.yml.restrictions.ids;
        }
        if (pi.yml.restrictions.max_length != "")
        {
            emitter << YAML::Key << "MAX_LENGTH";
            emitter << YAML::Value << pi.yml.restrictions.max_length;
        }
        emitter << YAML::EndMap;
    }

    emitter << YAML::EndMap;

    return true;
}

QWidget* MainWindow::CreatePropertiesWidget(QString type)
{
    QGridLayout* gridLayoutProperties = new QGridLayout;

    TabControls& tc = GetTabControls(type);

    int index = 0;
    AddLineEditRequiredProperty(gridLayoutProperties, "NAME", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditRequiredProperty(gridLayoutProperties, "TYPE", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "DISPLAY_NAME", index++, tc.Properties);
    AddPlainTextEditProperty(gridLayoutProperties, "DESCRIPTION", index++, tc.Properties);
    AddCheckBoxProperty(gridLayoutProperties, "REQUIRED", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "DEFAULT", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "HINT", index++, tc.Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (base)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "MAX", index++, tc.Properties);
    AddListProperty(gridLayoutProperties, "SET", index++, "PropertiesTab", "RestrictionsSet", tc.Properties, type);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (array)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN_COUNT", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "MAX_COUNT", index++, tc.Properties);
    AddListProperty(gridLayoutProperties, "SET_COUNT", index++, "PropertiesTab", "RestrictionsSetCount", tc.Properties, type);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (unit)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "CATEGORY", index++, tc.Properties);
    AddListProperty(gridLayoutProperties, "IDS", index++, "PropertiesTab", "RestrictionsIds", tc.Properties, type);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (path)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MAX_LENGTH", index++, tc.Properties);

    gridLayoutProperties->setRowStretch(gridLayoutProperties->rowCount(), 1);

    QWidget* widgetSplitterProperties = new QWidget;
    widgetSplitterProperties->setLayout(gridLayoutProperties);

    QScrollArea* scrollAreaProperties = new QScrollArea;
    scrollAreaProperties->setWidget(widgetSplitterProperties);
    scrollAreaProperties->setWidgetResizable(true);
    scrollAreaProperties->setFrameStyle(0);

    return scrollAreaProperties;
}

QWidget* MainWindow::CreateListControlWidget(int buttonSize, QString tabId, QString listControlId, QString type)
{
    QHBoxLayout* hBoxLayoutPropertyListButtons = new QHBoxLayout;
    hBoxLayoutPropertyListButtons->setMargin(0);

    QToolButton* toolButtonPropertyListAdd = new QToolButton;
    toolButtonPropertyListAdd->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListAdd->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListAdd->setIcon(QIcon(":/images/plus.png"));
    toolButtonPropertyListAdd->setProperty("tabId", tabId);
    toolButtonPropertyListAdd->setProperty("listControlId", listControlId);
    toolButtonPropertyListAdd->setProperty("action", "add");
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListAdd);
    connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);

    QToolButton* toolButtonPropertyListRemove = new QToolButton;
    toolButtonPropertyListRemove->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListRemove->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListRemove->setIcon(QIcon(":/images/minus.png"));
    toolButtonPropertyListRemove->setProperty("tabId", tabId);
    toolButtonPropertyListRemove->setProperty("listControlId", listControlId);
    toolButtonPropertyListRemove->setProperty("action", "remove");
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListRemove);
    connect(toolButtonPropertyListRemove, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);

    QToolButton* toolButtonPropertyListUp = new QToolButton;
    toolButtonPropertyListUp->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListUp->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListUp->setIcon(QIcon(":/images/up.png"));
    toolButtonPropertyListUp->setProperty("tabId", tabId);
    toolButtonPropertyListUp->setProperty("listControlId", listControlId);
    toolButtonPropertyListUp->setProperty("action", "up");
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListUp);
    connect(toolButtonPropertyListUp, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);

    QToolButton* toolButtonPropertyListDown = new QToolButton;
    toolButtonPropertyListDown->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListDown->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListDown->setIcon(QIcon(":/images/down.png"));
    toolButtonPropertyListDown->setProperty("tabId", tabId);
    toolButtonPropertyListDown->setProperty("listControlId", listControlId);
    toolButtonPropertyListDown->setProperty("action", "down");
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListDown);
    connect(toolButtonPropertyListDown, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);

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

    QSplitter* tabHSplitter = new QSplitter(Qt::Horizontal);
    tabHSplitter->addWidget(widgetSplitterInfo);
    tabHSplitter->addWidget(widgetSplitterPropertyList);
    tabHSplitter->addWidget(widgetSplitterProperties);
    tabHSplitter->setStretchFactor(0, 1);
    tabHSplitter->setStretchFactor(1, 0);
    tabHSplitter->setStretchFactor(2, 1);

    QVBoxLayout* vBoxLayoutSplitter = new QVBoxLayout;
    vBoxLayoutSplitter->addWidget(tabHSplitter);
    widgetTabProperties->setLayout(vBoxLayoutSplitter);

    return widgetTabProperties;
}

QWidget* MainWindow::CreateTypeTabInfoWidget(QString type)
{
    QGridLayout* gridLayoutInfo = new QGridLayout;

    TabControls& tc = GetTabControls(type);

    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "TYPES", "font-weight: bold; font-size: 14px", index++);
    AddLineEditRequiredProperty(gridLayoutInfo, "NAME", index++, type, MainWindow::ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "TYPE", index++, tc.Info);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, tc.Info);
    AddListProperty(gridLayoutInfo, "VALUES", index++, type, "Values", tc.Info, type);
    AddListProperty(gridLayoutInfo, "INCLUDES", index++, type, "Includes", tc.Info, type);

    //AddLineEditProperty(gridLayoutInfo, "VALUES", index++, tc.Info);
    //AddLineEditProperty(gridLayoutInfo, "INCLUDES", index++, tc.Info);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

void MainWindow::on_toolButton_clicked()
{
    QToolButton* tb = dynamic_cast<QToolButton*>(sender());
    if (tb)
    {
        qDebug() << tb->property("tabId").toString();
        qDebug() << tb->property("listControlId").toString();
        qDebug() << tb->property("action").toString();
    }
}

void MainWindow::on_toolButtonAddProperty_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Property", "Property name:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(text);
//        ui->listWidgetProperties->addItem(newItem);
    }
}

void MainWindow::on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QListWidget* list = dynamic_cast<QListWidget*>(sender());
    if (!list)
        return;

    QString type = list->property("type").toString();

    std::vector<yaml::parameter_info>* pis = &fileInfo_.parameters;
    if (type != "Main")
    {
        for (auto& t : fileInfo_.types)
            if (QString(t.yml.name.c_str()) == type)
            {
                pis = &t.parameters;
                break;
            }
    }

    yaml::parameter_info pic{};
    if (current != nullptr)
    {
        for (const auto& p : *pis)
            if (QString(p.yml.name.c_str()) == current->text())
            {
                pic = p;
                break;
            }
    }

    if (previous != nullptr)
    {
        yaml::parameter_info pip{};
        if (!ReadCurrentParameters(type, pip))
            return;

        for (auto& p : *pis)
            if (QString(p.yml.name.c_str()) == previous->text())
            {
                p = pip;
                break;
            }
    }

    TabControls& tc = GetTabControls(type);

    dynamic_cast<QLineEdit*>(tc.Properties["NAME"])->setText(QString(pic.yml.name.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["TYPE"])->setText(QString(pic.yml.type.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->setText(QString(pic.yml.display_name.c_str()));
    dynamic_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->setPlainText(QString(pic.yml.description.c_str()));
    dynamic_cast<QCheckBox*>(tc.Properties["REQUIRED"])->setChecked(pic.yml.required);
    dynamic_cast<QLineEdit*>(tc.Properties["DEFAULT"])->setText(QString(pic.yml.default_.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["HINT"])->setText(QString(pic.yml.hint.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["MIN"])->setText(QString(pic.yml.restrictions.min.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["MAX"])->setText(QString(pic.yml.restrictions.max.c_str()));
    QListWidget* listWidgetSet = dynamic_cast<QListWidget*>(tc.Properties["SET"]);
    listWidgetSet->clear();
    for (const auto& s : pic.yml.restrictions.set_)
        listWidgetSet->addItem(QString(s.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->setText(QString(pic.yml.restrictions.min_count.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->setText(QString(pic.yml.restrictions.max_count.c_str()));
    QListWidget* listWidgetSetCount = dynamic_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    listWidgetSetCount->clear();
    for (const auto& s : pic.yml.restrictions.set_count)
        listWidgetSetCount->addItem(QString(s.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["CATEGORY"])->setText(QString(pic.yml.restrictions.category.c_str()));
    QListWidget* listWidgetIds = dynamic_cast<QListWidget*>(tc.Properties["IDS"]);
    listWidgetIds->clear();
    for (const auto& s : pic.yml.restrictions.ids)
        listWidgetIds->addItem(QString(s.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->setText(QString(pic.yml.restrictions.max_length.c_str()));
}


void MainWindow::on_toolButtonRemoveProperty_clicked()
{
//    auto si = ui->listWidgetProperties->selectedItems();
//    if (si.size() > 0)
//    {
//        delete si[0];
//    }
}

void MainWindow::on_editingFinished()
{
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(sender());
    if (!lineEdit->isModified()) return; //!!!
    lineEdit->setModified(false);
    qDebug() << lineEdit->text();

    QString name = lineEdit->property("name").toString();
    MainWindow::ControlsGroup group = static_cast<MainWindow::ControlsGroup>(lineEdit->property("group").toInt());
    QString type = lineEdit->property("type").toString();
    if (name == "" || type == "")
        return;

    TabControls& tc = GetTabControls(type);

    if (group == MainWindow::ControlsGroup::Info && name == "ID")
    {
        // Main ID
    }
    else if (group == MainWindow::ControlsGroup::Info && name == "NAME")
    {
        // Type NAME
        QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(centralWidget());

        int index = tabWidget->currentIndex();
        QString oldName = tabWidget->tabText(index);

        for (const auto& ti : fileInfo_.types)
        {
            if (QString::fromStdString(ti.yml.name) == lineEdit->text())
            {
                QMessageBox::critical(this, "Error", "Type with this NAME already exists: " + lineEdit->text());
                lineEdit->setText(oldName);
                return;
            }
        }
    }
    else if (group == MainWindow::ControlsGroup::Properties && name == "NAME")
    {
        // Property NAME
        QListWidget* listWidget = dynamic_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
        if (listWidget->selectedItems().size() == 0) return; // !!!

        QString oldName = listWidget->selectedItems()[0]->text();

        for (int i = 0; i < listWidget->count(); ++i)
        {
            if (listWidget->item(i)->text() == lineEdit->text() && listWidget->selectedItems()[0] != listWidget->item(i))
            {
                QMessageBox::critical(this, "Error", "Property with this NAME already exists: " + lineEdit->text());
                if (listWidget->selectedItems().size() > 0)
                    lineEdit->setText(oldName);
                return;
            }
        }

        listWidget->selectedItems()[0]->setText(lineEdit->text());

        if (type == "Main")
        {
            for (auto& p : fileInfo_.parameters)
            {
                if (QString(p.yml.name.c_str()) == oldName)
                {
                    p.yml.name = lineEdit->text().toStdString();
                    break;
                }
            }
        }
        else
        {
            for (auto& t : fileInfo_.types)
            {
                if (QString(t.yml.name.c_str()) == type)
                {
                    for (auto& p : t.parameters)
                    {
                        if (QString(p.yml.name.c_str()) == oldName)
                        {
                            p.yml.name = lineEdit->text().toStdString();
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    else if (group == MainWindow::ControlsGroup::Properties && name == "TYPE")
    {
        // Property TYPE
    }
}
