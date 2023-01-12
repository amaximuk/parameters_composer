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
    currentFileName_ = "";
    fileInfo_ = {};

    CreateUi();

    setWindowIcon(QIcon(":/images/parameters.png"));
    setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        "Are you sure want to exit?\nAll unsaved work will be lost!", QMessageBox::No | QMessageBox::Yes);
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
    case MainWindow::ControlsGroup::PropertyList:
        return tc.PropertyList;
    case MainWindow::ControlsGroup::Properties:
        return tc.Properties;
    default:
        return tc.Info; // fake
    }
}

bool MainWindow::RenameTabControls(QString oldType, QString newType)
{
    TabControls& tc = GetTabControls(oldType);
    
    tc.Name = newType;

    for (auto& x : tc.Info)
        x->setProperty("type", newType);
    for (auto& x : tc.PropertyList)
        x->setProperty("type", newType);
    for (auto& x : tc.Properties)
        x->setProperty("type", newType);
 

    return true;
}

void MainWindow::on_NewFile_action()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        "Are you sure want to create new file?\nAll unsaved work will be lost!", QMessageBox::No | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
        return;

    while (qobject_cast<QTabWidget*>(centralWidget())->count() > 0)
        qobject_cast<QTabWidget*>(centralWidget())->removeTab(0);

    QWidget* widgetTabProperties = CreateMainTabWidget();
    qobject_cast<QTabWidget*>(centralWidget())->addTab(widgetTabProperties, "Main");

    fileInfo_ = {};

    currentFileName_ = "";
    setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);
}

void MainWindow::on_Quit_action()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        "Are you sure want to exit?\nAll unsaved work will be lost!", QMessageBox::No | QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
        QApplication::quit();
}

void MainWindow::on_OpenFile_action()
{
    if (currentFileName_ != "")
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            "Are you sure want to open file?\nAll unsaved work will be lost!", QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;
    }

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
            setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);
            fileInfo_ = parser.get_file_info();

            {
                TabControls& tc = GetTabControls("Main");

                qobject_cast<QLineEdit*>(tc.Info["ID"])->setText(QString::fromStdString(fileInfo_.info.yml.id));
                qobject_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->setText(QString::fromStdString(fileInfo_.info.yml.display_name));
                qobject_cast<QLineEdit*>(tc.Info["CATEGORY"])->setText(QString::fromStdString(fileInfo_.info.yml.category));
                qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString::fromStdString(fileInfo_.info.yml.description));
                qobject_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->setText(QString::fromStdString(fileInfo_.info.yml.pictogram));
                qobject_cast<QLineEdit*>(tc.Info["HINT"])->setText(QString::fromStdString(fileInfo_.info.yml.hint));
                qobject_cast<QLineEdit*>(tc.Info["AUTHOR"])->setText(QString::fromStdString(fileInfo_.info.yml.author));
                qobject_cast<QLineEdit*>(tc.Info["WIKI"])->setText(QString::fromStdString(fileInfo_.info.yml.wiki));
                qobject_cast<QLineEdit*>(tc.Info["WIKI"])->setCursorPosition(0);

                QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
                listWidget->clear();
                for (const auto& pi : fileInfo_.parameters)
                    listWidget->addItem(QString::fromStdString(pi.yml.name));
                if (listWidget->count() > 0)
                    listWidget->setCurrentRow(0);
            }

            while (qobject_cast<QTabWidget*>(centralWidget())->count() > 1)
                qobject_cast<QTabWidget*>(centralWidget())->removeTab(1);

            for (const auto& ti : fileInfo_.types)
            {
                QString type = QString::fromStdString(ti.yml.name);

                QWidget* widgetTabType = CreateTypeTabWidget(type);
                qobject_cast<QTabWidget*>(centralWidget())->addTab(widgetTabType, type);
                
                TabControls& tc = GetTabControls(type);

                qobject_cast<QLineEdit*>(tc.Info["NAME"])->setText(QString::fromStdString(ti.yml.name));
                qobject_cast<QLineEdit*>(tc.Info["TYPE"])->setText(QString::fromStdString(ti.yml.type));
                qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString::fromStdString(ti.yml.description));

                QListWidget* listWidgetValues = qobject_cast<QListWidget*>(tc.Info["VALUES"]);
                listWidgetValues->clear();
                for (const auto& v : ti.yml.values)
                    listWidgetValues->addItem(QString::fromStdString(v.first) + " -> " + QString::fromStdString(v.second));

                QListWidget* listWidgetIncludes = qobject_cast<QListWidget*>(tc.Info["INCLUDES"]);
                listWidgetIncludes->clear();
                for (const auto& v : ti.yml.includes)
                    listWidgetIncludes->addItem(QString::fromStdString(v));

                QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
                listWidget->clear();
                for (const auto& pi : ti.parameters)
                    listWidget->addItem(QString::fromStdString(pi.yml.name));
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
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        "Are you sure want to save file?\nFile will be overwriten!", QMessageBox::No | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
        return;

    QStringList fileNames;
    if (currentFileName_ == "")
    {
        QFileDialog dialog(this);
        dialog.setNameFilter("Parameters Compiler Files (*.yml)");
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setDefaultSuffix("yml");

        if (dialog.exec())
            fileNames = dialog.selectedFiles();
    }
    else
        fileNames.push_back(currentFileName_);

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
            file.write(emitter.c_str());

        currentFileName_ = fileNames[0];
        setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);
    }
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
            file.write(emitter.c_str());

        currentFileName_ = fileNames[0];
        setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);
    }
}

void MainWindow::on_AddType_action()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add type", "Type name:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        for (const auto& ti : fileInfo_.types)
        {
            if (QString::fromStdString(ti.yml.name) == text)
            {
                QMessageBox::critical(this, "Error", "Type with this NAME already exists: " + text);
                return;
            }
        }

        QWidget* widgetTabType = CreateTypeTabWidget(text);
        qobject_cast<QTabWidget*>(centralWidget())->addTab(widgetTabType, text);
        qobject_cast<QTabWidget*>(centralWidget())->setCurrentWidget(widgetTabType);

        TabControls& tc = GetTabControls(text);
        qobject_cast<QLineEdit*>(tc.Info["NAME"])->setText(text);

        yaml::type_info ti{};
        ti.yml.name = text.toStdString();
        fileInfo_.types.push_back(ti);
    }
}

void MainWindow::on_RemoveType_action()
{
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget());
    QString name = tabWidget->tabText(tabWidget->indexOf(tabWidget->currentWidget()));

    if (name == "Main")
    {
        QMessageBox::warning(this, "parameters_composer", "Main page cannot be deleted");
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        QString("Are you sure want to delete type: %1?").arg(name), QMessageBox::No | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
        return;

    QList<QString> usedInTypes;
    QString arrayName = QString("array<%1>").arg(name);

    // Update fileInfo_
    for (auto& p : fileInfo_.parameters)
    {
        if (QString::fromStdString(p.yml.type) == name || QString::fromStdString(p.yml.type) == arrayName)
        {
            usedInTypes.push_back("Main");
            break;
        }
    }
    for (auto& t : fileInfo_.types)
    {
        for (auto& p : t.parameters)
        {
            if (QString::fromStdString(p.yml.type) == name || QString::fromStdString(p.yml.type) == arrayName)
            {
                usedInTypes.push_back(QString::fromStdString(t.yml.name));
                break;
            }
        }
    }

    if (usedInTypes.size() > 0)
    {
        QString message = QString("Type %1 is used in other types:\n").arg(name);
        for (const auto& s : usedInTypes)
            message += s + "\n";
        QMessageBox::StandardButton resBtn = QMessageBox::critical(this, "parameters_composer", message);
        return;
    }

    tabWidget->removeTab(tabWidget->indexOf(tabWidget->currentWidget()));
    auto it = std::find_if(fileInfo_.types.cbegin(), fileInfo_.types.cend(), [name](auto& t) { if (t.yml.name == name.toStdString()) return true; else return false; });
    fileInfo_.types.erase(it);
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

    QAction* addTypeAct = new QAction(tr("&Add"), this);
    addTypeAct->setStatusTip(tr("Add type"));
    connect(addTypeAct, &QAction::triggered, this, &MainWindow::on_AddType_action);

    QAction* removeTypeAct = new QAction(tr("&Remove"), this);
    removeTypeAct->setStatusTip(tr("Remove type"));
    connect(removeTypeAct, &QAction::triggered, this, &MainWindow::on_RemoveType_action);

    QMenu* typeMenu = menuBar()->addMenu("&Types");
    typeMenu->addAction(addTypeAct);
    typeMenu->addAction(removeTypeAct);
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

    AddGroupWidget(widgetSplitterInfo, "INFO_GROUP", "Main", ControlsGroup::Info);
    AddGroupWidget(widgetSplitterPropertyList, "PROPERTY_LIST_GROUP", "Main", ControlsGroup::PropertyList);
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
    AddLineEditRequiredProperty(gridLayoutInfo, "ID", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "DISPLAY_NAME", index++, type, ControlsGroup::Info);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "CATEGORY", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "PICTOGRAM", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "HINT", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "AUTHOR", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "WIKI", index++, type, ControlsGroup::Info);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

QWidget* MainWindow::CreatePropertyListWidget(QString type)
{
    QLabel* labelPropertyListHeader = new QLabel;
    labelPropertyListHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelPropertyListHeader->setText("PROPERTIES");

    QWidget* widgetPropertyListButtons = CreateListControlWidget(32, type, ControlsGroup::PropertyList, "PROPERTIES");

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

    auto& tc = GetControls(type, ControlsGroup::PropertyList);
    tc["PROPERTIES"] = listWidget;

    return widgetSplitterPropertyList;
}

void MainWindow::AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QLineEdit* lineEdit = new QLineEdit;
    gridLayout->addWidget(lineEdit, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = lineEdit;
}

void MainWindow::AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QPlainTextEdit* plainTextEdit = new QPlainTextEdit;
    gridLayout->addWidget(plainTextEdit, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = plainTextEdit;
}

void MainWindow::AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QCheckBox* checkBox = new QCheckBox;
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

//QComboBox

//void MainWindow::AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId, QMap<QString, QObject*>& mapControls, QString type)
void MainWindow::AddListProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group)
{
    QWidget* widgetPropertiesRestrictionsSetButtons = CreateListControlWidget(24, type, group, name);
    QListWidget* listWidget = new QListWidget;
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
    
    //connect(lineEdit, &QLineEdit::editingFinished, this, std::bind(&MainWindow::OnEditingFinished, this, type, group, name));
    connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::on_editingFinished);
    gridLayout->addWidget(lineEdit, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = lineEdit;
}

void MainWindow::AddGroupWidget(QWidget* groupWidget, QString name, QString type, ControlsGroup group)
{
    auto& tc = GetControls(type, group);
    tc[name] = groupWidget;
}

bool MainWindow::ReadCurrentParameters(QString type, yaml::parameter_info& pi)
{
    TabControls& tc = GetTabControls(type);

    pi.yml.name = qobject_cast<QLineEdit*>(tc.Properties["NAME"])->text().toStdString();
    pi.yml.type = qobject_cast<QLineEdit*>(tc.Properties["TYPE"])->text().toStdString();
    pi.yml.display_name = qobject_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->text().toStdString();
    pi.yml.description = qobject_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->toPlainText().toStdString();
    pi.yml.required = qobject_cast<QCheckBox*>(tc.Properties["REQUIRED"])->isChecked();
    pi.yml.default_ = qobject_cast<QLineEdit*>(tc.Properties["DEFAULT"])->text().toStdString();
    pi.yml.hint = qobject_cast<QLineEdit*>(tc.Properties["HINT"])->text().toStdString();

    pi.yml.restrictions.min = qobject_cast<QLineEdit*>(tc.Properties["MIN"])->text().toStdString();
    pi.yml.restrictions.max = qobject_cast<QLineEdit*>(tc.Properties["MAX"])->text().toStdString();
    pi.yml.restrictions.set_.clear();
    QListWidget* listWidgetSet = qobject_cast<QListWidget*>(tc.Properties["SET"]);
    for (int i = 0; i < listWidgetSet->count(); ++i)
        pi.yml.restrictions.set_.push_back(listWidgetSet->item(i)->text().toStdString());

    pi.yml.restrictions.min_count = qobject_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->text().toStdString();
    pi.yml.restrictions.max_count = qobject_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->text().toStdString();
    pi.yml.restrictions.set_count.clear();
    QListWidget* listWidgetSetCount = qobject_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    for (int i = 0; i < listWidgetSetCount->count(); ++i)
        pi.yml.restrictions.set_count.push_back(listWidgetSetCount->item(i)->text().toStdString());

    pi.yml.restrictions.category = qobject_cast<QLineEdit*>(tc.Properties["CATEGORY"])->text().toStdString();

    pi.yml.restrictions.ids.clear();
    QListWidget* listWidgetIds = qobject_cast<QListWidget*>(tc.Properties["IDS"]);
    for (int i = 0; i < listWidgetIds->count(); ++i)
        pi.yml.restrictions.ids.push_back(listWidgetIds->item(i)->text().toStdString());

    pi.yml.restrictions.max_length = qobject_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->text().toStdString();

    return true;
}

bool MainWindow::ReadCurrentMainInfo(QString type, yaml::info_info& mi)
{
    TabControls& tc = GetTabControls("Main");

    mi.yml.id = qobject_cast<QLineEdit*>(tc.Info["ID"])->text().toStdString();
    mi.yml.display_name = qobject_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->text().toStdString();
    mi.yml.description = qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->toPlainText().toStdString();
    mi.yml.category = qobject_cast<QLineEdit*>(tc.Info["CATEGORY"])->text().toStdString();
    mi.yml.pictogram = qobject_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->text().toStdString();
    mi.yml.hint = qobject_cast<QLineEdit*>(tc.Info["HINT"])->text().toStdString();
    mi.yml.author = qobject_cast<QLineEdit*>(tc.Info["AUTHOR"])->text().toStdString();
    mi.yml.wiki = qobject_cast<QLineEdit*>(tc.Info["WIKI"])->text().toStdString();

    return true;
}

bool MainWindow::ReadCurrentTypeInfo(QString type, yaml::type_info& ti)
{
    TabControls& tc = GetTabControls(type);

    ti.yml.name = qobject_cast<QLineEdit*>(tc.Info["NAME"])->text().toStdString();
    ti.yml.type = qobject_cast<QLineEdit*>(tc.Info["TYPE"])->text().toStdString();
    ti.yml.description = qobject_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->toPlainText().toStdString();

    ti.yml.values.clear();
    QListWidget* listWidgetValues = qobject_cast<QListWidget*>(tc.Info["VALUES"]);
    for (int i = 0; i < listWidgetValues->count(); ++i)
    {
        QString v = listWidgetValues->item(i)->text();
        QStringList sl = v.split(" -> ");
        ti.yml.values.push_back({ sl[0].toStdString(), sl[1].toStdString() });
    }

    ti.yml.includes.clear();
    QListWidget* listWidgetIncludes = qobject_cast<QListWidget*>(tc.Info["INCLUDES"]);
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
            if (QString::fromStdString(t.yml.name) == type)
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
            if (QString::fromStdString(t.yml.name) == type)
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
        if (QString::fromStdString(ti.yml.name) == type->Name)
        {
            for (const auto& pi : ti.parameters)
            {
                QString ts = QString::fromStdString(pi.yml.type);
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
            QString tn = QString::fromStdString(ti.yml.name);
            if (sorted_names.contains(tn))
                continue;

            bool have_unresolved = false;
            for (const auto& pi : ti.parameters)
            {
                QString ts = QString::fromStdString(pi.yml.type);
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
            QString tn = QString::fromStdString(ti.yml.name);
            if (!sorted_names.contains(tn))
                sorted_names.push_back(tn);
        }
    }
  
    std::vector<yaml::type_info> sorted_types;
    for (const auto& sn : sorted_names)
    {
        for (const auto& ti : fileInfo_.types)
        {
            if (sn == QString::fromStdString(ti.yml.name))
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

    int index = 0;
    AddLineEditRequiredProperty(gridLayoutProperties, "NAME", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditRequiredProperty(gridLayoutProperties, "TYPE", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "DISPLAY_NAME", index++, type, MainWindow::ControlsGroup::Properties);
    AddPlainTextEditProperty(gridLayoutProperties, "DESCRIPTION", index++, type, MainWindow::ControlsGroup::Properties);
    AddCheckBoxProperty(gridLayoutProperties, "REQUIRED", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "DEFAULT", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "HINT", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (base)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "MAX", index++, type, MainWindow::ControlsGroup::Properties);
    AddListProperty(gridLayoutProperties, "SET", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (array)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN_COUNT", index++, type, MainWindow::ControlsGroup::Properties);
    AddLineEditProperty(gridLayoutProperties, "MAX_COUNT", index++, type, MainWindow::ControlsGroup::Properties);
    AddListProperty(gridLayoutProperties, "SET_COUNT", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (unit)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "CATEGORY", index++, type, MainWindow::ControlsGroup::Properties);
    AddListProperty(gridLayoutProperties, "IDS", index++, type, MainWindow::ControlsGroup::Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (path)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MAX_LENGTH", index++, type, MainWindow::ControlsGroup::Properties);

    gridLayoutProperties->setRowStretch(gridLayoutProperties->rowCount(), 1);

    QWidget* widgetSplitterProperties = new QWidget;
    widgetSplitterProperties->setLayout(gridLayoutProperties);

    QScrollArea* scrollAreaProperties = new QScrollArea;
    scrollAreaProperties->setWidget(widgetSplitterProperties);
    scrollAreaProperties->setWidgetResizable(true);
    scrollAreaProperties->setFrameStyle(0);

    return scrollAreaProperties;
}

QWidget* MainWindow::CreateListControlWidget(int buttonSize, QString type, ControlsGroup group, QString name)
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
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListAdd);
    connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);
    //connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, std::bind(&MainWindow::on_toolButton_name_clicked, this, QString("Knopka")));

    QToolButton* toolButtonPropertyListRemove = new QToolButton;
    toolButtonPropertyListRemove->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListRemove->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListRemove->setIcon(QIcon(":/images/minus.png"));
    toolButtonPropertyListRemove->setProperty("type", type);
    toolButtonPropertyListRemove->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListRemove->setProperty("name", name);
    toolButtonPropertyListRemove->setProperty("action", "remove");
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListRemove);
    connect(toolButtonPropertyListRemove, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);

    QToolButton* toolButtonPropertyListUp = new QToolButton;
    toolButtonPropertyListUp->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListUp->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListUp->setIcon(QIcon(":/images/up.png"));
    toolButtonPropertyListUp->setProperty("type", type);
    toolButtonPropertyListUp->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListUp->setProperty("name", name);
    toolButtonPropertyListUp->setProperty("action", "up");
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListUp);
    connect(toolButtonPropertyListUp, &QToolButton::clicked, this, &MainWindow::on_toolButton_clicked);

    QToolButton* toolButtonPropertyListDown = new QToolButton;
    toolButtonPropertyListDown->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListDown->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListDown->setIcon(QIcon(":/images/down.png"));
    toolButtonPropertyListDown->setProperty("type", type);
    toolButtonPropertyListDown->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListDown->setProperty("name", name);
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

    AddGroupWidget(widgetSplitterInfo, "INFO_GROUP", type, ControlsGroup::Info);
    AddGroupWidget(widgetSplitterPropertyList, "PROPERTY_LIST_GROUP", type, ControlsGroup::PropertyList);
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
    AddLineEditRequiredProperty(gridLayoutInfo, "NAME", index++, type, ControlsGroup::Info);
    AddLineEditProperty(gridLayoutInfo, "TYPE", index++, type, ControlsGroup::Info);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, type, ControlsGroup::Info);
    AddListProperty(gridLayoutInfo, "VALUES", index++, type, ControlsGroup::Info);
    AddListProperty(gridLayoutInfo, "INCLUDES", index++, type, ControlsGroup::Info);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

void MainWindow::on_toolButton_clicked()
{
    QToolButton* tb = qobject_cast<QToolButton*>(sender());
    QString type = tb->property("type").toString();
    ControlsGroup group = static_cast<ControlsGroup>(tb->property("group").toInt());
    QString name = tb->property("name").toString();
    QString action = tb->property("action").toString();

    if (group == ControlsGroup::PropertyList && name == "PROPERTIES" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add property", "Property name:", QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty())
        {
            QListWidgetItem* newItem = new QListWidgetItem;
            newItem->setText(text);

            // Validate
            if (type == "Main")
            {
                for (auto& p : fileInfo_.parameters)
                {
                    if (QString::fromStdString(p.yml.name) == text)
                    {
                        QMessageBox::critical(this, "Error", "Parameter with this NAME already exists: " + text);
                        return;
                    }
                }
            }
            else
            {
                for (auto& t : fileInfo_.types)
                {
                    if (QString::fromStdString(t.yml.name) == type)
                    {
                        for (auto& p : t.parameters)
                        {
                            if (QString::fromStdString(p.yml.name) == text)
                            {
                                QMessageBox::critical(this, "Error", "Parameter with this NAME already exists: " + text);
                                return;
                            }
                        }
                        break;
                    }
                }
            }

            // Add to control
            auto& tc = GetControls(type, group);
            QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
            listWidget->addItem(newItem);

            // Add to fileInfo_
            if (type == "Main")
            {
                yaml::parameter_info pi{};
                pi.yml.name = text.toStdString();
                pi.yml.type = "string";
                fileInfo_.parameters.push_back(pi);
            }
            else
            {
                for (auto& t : fileInfo_.types)
                {
                    if (QString::fromStdString(t.yml.name) == type)
                    {
                        yaml::parameter_info pi{};
                        pi.yml.name = text.toStdString();
                        pi.yml.type = "string";
                        t.parameters.push_back(pi);
                        break;
                    }
                }
            }

            // Update
            listWidget->setCurrentRow(listWidget->count() - 1);
        }
    }
    else if (group == ControlsGroup::PropertyList && name == "PROPERTIES" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
        if (listWidget->currentItem() == nullptr)
            return;

        QString propertyName = listWidget->currentItem()->text();

        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
            QString("Are you sure want to remove property: %1?").arg(propertyName), QMessageBox::No | QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
            return;

        // Remove from control
        listWidget->removeItemWidget(listWidget->currentItem());
        delete listWidget->currentItem();

        // Remove from fileInfo_
        if (type == "Main")
        {
            auto it = std::find_if(fileInfo_.parameters.cbegin(), fileInfo_.parameters.cend(), [propertyName](auto& p) { if (p.yml.name == propertyName.toStdString()) return true; else return false; });
            fileInfo_.parameters.erase(it);
        }
        else
        {
            for (auto& t : fileInfo_.types)
            {
                if (QString::fromStdString(t.yml.name) == type)
                {
                    auto it = std::find_if(t.parameters.cbegin(), t.parameters.cend(), [propertyName](auto& p) { if (p.yml.name == propertyName.toStdString()) return true; else return false; });
                    t.parameters.erase(it);
                    break;
                }
            }
        }
    }
    else if (group == ControlsGroup::Properties && name == "SET" && action == "add")
    {
        auto& tc = GetControls(type, ControlsGroup::PropertyList);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
        if (listWidget->currentItem() == nullptr)
            return;

        QString propertyName = listWidget->currentItem()->text();

        bool ok;
        QString text = QInputDialog::getText(this, "Add restriction SET", "Value:", QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty())
        {
            QListWidgetItem* newItem = new QListWidgetItem;
            newItem->setText(text);

            // Validate
            if (type == "Main")
            {
                for (auto& p : fileInfo_.parameters)
                {
                    if (QString::fromStdString(p.yml.name) == propertyName)
                    {
                        auto it = std::find_if(p.yml.restrictions.set_.cbegin(), p.yml.restrictions.set_.cend(), [text](auto& p) { if (p == text.toStdString()) return true; else return false; });
                        if (it != p.yml.restrictions.set_.cend())
                        {
                            QMessageBox::critical(this, "Error", "Restriction with this value already exists: " + text);
                            return;
                        }
                    }
                }
            }
            else
            {
                for (auto& t : fileInfo_.types)
                {
                    if (QString::fromStdString(t.yml.name) == type)
                    {
                        for (auto& p : t.parameters)
                        {
                            if (QString::fromStdString(p.yml.name) == propertyName)
                            {
                                auto it = std::find_if(p.yml.restrictions.set_.cbegin(), p.yml.restrictions.set_.cend(), [text](auto& p) { if (p == text.toStdString()) return true; else return false; });
                                if (it != p.yml.restrictions.set_.cend())
                                {
                                    QMessageBox::critical(this, "Error", "Restriction with this value already exists: " + text);
                                    return;
                                }
                            }
                        }
                        break;
                    }
                }
            }

            // Add to control
            auto& tc = GetControls(type, group);
            QListWidget* listWidget = qobject_cast<QListWidget*>(tc[name]);
            listWidget->addItem(newItem);

            // Add to fileInfo_
            if (type == "Main")
            {
                for (auto& p : fileInfo_.parameters)
                {
                    if (QString::fromStdString(p.yml.name) == propertyName)
                    {
                        p.yml.restrictions.set_.push_back(text.toStdString());
                        break;
                    }
                }
            }
            else
            {
                for (auto& t : fileInfo_.types)
                {
                    if (QString::fromStdString(t.yml.name) == type)
                    {
                        for (auto& p : t.parameters)
                        {
                            if (QString::fromStdString(p.yml.name) == propertyName)
                            {
                                p.yml.restrictions.set_.push_back(text.toStdString());
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
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
    QListWidget* list = qobject_cast<QListWidget*>(sender());
    QString type = list->property("type").toString();

    std::vector<yaml::parameter_info>* pis = &fileInfo_.parameters;
    if (type != "Main")
    {
        for (auto& t : fileInfo_.types)
            if (QString::fromStdString(t.yml.name) == type)
            {
                pis = &t.parameters;
                break;
            }
    }

    yaml::parameter_info pic{};
    if (current != nullptr)
    {
        for (const auto& p : *pis)
            if (QString::fromStdString(p.yml.name) == current->text())
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
            if (QString::fromStdString(p.yml.name) == previous->text())
            {
                p = pip;
                break;
            }
    }

    TabControls& tc = GetTabControls(type);

    qobject_cast<QLineEdit*>(tc.Properties["NAME"])->setText(QString::fromStdString(pic.yml.name));
    qobject_cast<QLineEdit*>(tc.Properties["TYPE"])->setText(QString::fromStdString(pic.yml.type));
    qobject_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->setText(QString::fromStdString(pic.yml.display_name));
    qobject_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->setPlainText(QString::fromStdString(pic.yml.description));
    qobject_cast<QCheckBox*>(tc.Properties["REQUIRED"])->setChecked(pic.yml.required);
    qobject_cast<QLineEdit*>(tc.Properties["DEFAULT"])->setText(QString::fromStdString(pic.yml.default_));
    qobject_cast<QLineEdit*>(tc.Properties["HINT"])->setText(QString::fromStdString(pic.yml.hint));

    qobject_cast<QLineEdit*>(tc.Properties["MIN"])->setText(QString::fromStdString(pic.yml.restrictions.min));
    qobject_cast<QLineEdit*>(tc.Properties["MAX"])->setText(QString::fromStdString(pic.yml.restrictions.max));
    QListWidget* listWidgetSet = qobject_cast<QListWidget*>(tc.Properties["SET"]);
    listWidgetSet->clear();
    for (const auto& s : pic.yml.restrictions.set_)
        listWidgetSet->addItem(QString::fromStdString(s));

    qobject_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->setText(QString::fromStdString(pic.yml.restrictions.min_count));
    qobject_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->setText(QString::fromStdString(pic.yml.restrictions.max_count));
    QListWidget* listWidgetSetCount = qobject_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    listWidgetSetCount->clear();
    for (const auto& s : pic.yml.restrictions.set_count)
        listWidgetSetCount->addItem(QString::fromStdString(s));

    qobject_cast<QLineEdit*>(tc.Properties["CATEGORY"])->setText(QString::fromStdString(pic.yml.restrictions.category));
    QListWidget* listWidgetIds = qobject_cast<QListWidget*>(tc.Properties["IDS"]);
    listWidgetIds->clear();
    for (const auto& s : pic.yml.restrictions.ids)
        listWidgetIds->addItem(QString::fromStdString(s));

    qobject_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->setText(QString::fromStdString(pic.yml.restrictions.max_length));

    if (current == nullptr)
    {
        qobject_cast<QWidget*>(tc.Properties["PROPERTIES_GROUP"])->setEnabled(false);
    }
    else
    {
        qobject_cast<QWidget*>(tc.Properties["PROPERTIES_GROUP"])->setEnabled(true);
    }
}


void MainWindow::on_toolButtonRemoveProperty_clicked()
{
//    auto si = ui->listWidgetProperties->selectedItems();
//    if (si.size() > 0)
//    {
//        delete si[0];
//    }
}

//void MainWindow::OnEditingFinished(QString type, ControlsGroup group, QString name)
void MainWindow::on_editingFinished()
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
        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget());

        QString oldName = type;
        QString newName = lineEdit->text();

        for (const auto& ti : fileInfo_.types)
        {
            if (QString::fromStdString(ti.yml.name) == newName)
            {
                QMessageBox::critical(this, "Error", "Type with this NAME already exists: " + newName);
                lineEdit->setText(oldName);
                return;
            }
        }

        // Update controls
        for (int i = 0; i < tabWidget->count(); i++)
        {
            if (tabWidget->tabText(i) == oldName)
            {
                tabWidget->setTabText(i, newName);
                break;
            }
        }

        QString arrayOldName = QString("array<%1>").arg(oldName);
        QString arrayNewName = QString("array<%1>").arg(newName);


        TabControls& tc_m = GetTabControls("Main");
        QLineEdit* t_m = qobject_cast<QLineEdit*>(tc_m.Properties["TYPE"]);
        if (t_m->text() == oldName)
        {
            t_m->setText(newName);
        }
        if (t_m->text() == arrayOldName)
        {
            t_m->setText(arrayNewName);
        }

        for (auto& t : fileInfo_.types)
        {
            TabControls& tc_t = GetTabControls(QString::fromStdString(t.yml.name));
            QLineEdit* t_t = qobject_cast<QLineEdit*>(tc_m.Properties["TYPE"]);
            if (t_t->text() == oldName)
            {
                t_t->setText(newName);
            }
            if (t_t->text() == arrayOldName)
            {
                t_t->setText(arrayNewName);
            }
        }

        RenameTabControls(oldName, newName);

        // Update fileInfo_
        for (auto& t : fileInfo_.types)
        {
            if (QString::fromStdString(t.yml.name) == type)
            {
                t.yml.name = newName.toStdString();
                break;
            }
        }

        for (auto& p : fileInfo_.parameters)
        {
            if (QString::fromStdString(p.yml.type) == oldName)
            {
                p.yml.type = newName.toStdString();
                break;
            }
            if (QString::fromStdString(p.yml.type) == arrayOldName)
            {
                p.yml.type = arrayNewName.toStdString();
                break;
            }
        }
        for (auto& t : fileInfo_.types)
        {
            for (auto& p : t.parameters)
            {
                if (QString::fromStdString(p.yml.type) == oldName)
                {
                    p.yml.type = newName.toStdString();
                    break;
                }
                if (QString::fromStdString(p.yml.type) == arrayOldName)
                {
                    p.yml.type = arrayNewName.toStdString();
                    break;
                }
            }
            break;
        }
    }
    else if (group == MainWindow::ControlsGroup::Properties && name == "NAME")
    {
        // Property NAME
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
        if (listWidget->selectedItems().size() == 0) return; // !!!

        QString oldName = listWidget->selectedItems()[0]->text();
        QString newName = lineEdit->text();

        for (int i = 0; i < listWidget->count(); ++i)
        {
            if (listWidget->item(i)->text() == newName && listWidget->selectedItems()[0] != listWidget->item(i))
            {
                QMessageBox::critical(this, "Error", "Property with this NAME already exists: " + newName);
                if (listWidget->selectedItems().size() > 0)
                    lineEdit->setText(oldName);
                return;
            }
        }

        listWidget->selectedItems()[0]->setText(newName);

        if (type == "Main")
        {
            for (auto& p : fileInfo_.parameters)
            {
                if (QString::fromStdString(p.yml.name) == oldName)
                {
                    p.yml.name = newName.toStdString();
                    break;
                }
            }
        }
        else
        {
            for (auto& t : fileInfo_.types)
            {
                if (QString::fromStdString(t.yml.name) == type)
                {
                    for (auto& p : t.parameters)
                    {
                        if (QString::fromStdString(p.yml.name) == oldName)
                        {
                            p.yml.name = newName.toStdString();
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
