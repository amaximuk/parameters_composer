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

#include "yaml_parser.h"
#include "yaml_writer.h"
#include "yaml_helper.h"
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
        QString::fromLocal8Bit("Вы действительно хотите выйти?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
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
        QString::fromLocal8Bit("Вы действительно хотите создать новый файл?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
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
        QString::fromLocal8Bit("Вы действительно хотите выйти?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
        QApplication::quit();
}

void MainWindow::on_OpenFile_action()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        QString::fromLocal8Bit("Вы действительно хотите открыть файл?\nВсе несохраненные изменения будут потеряны!"), QMessageBox::No | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
        return;

    QFileDialog dialog(this);
    dialog.setNameFilter("Parameters Compiler Files (*.yml *.yaml *.json)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() > 0)
    {
        qDebug() << fileNames[0];
        yaml::file_info fi{};
        if (yaml::parser::parse(fileNames[0].toStdString(), fi))
        {
            while (qobject_cast<QTabWidget*>(centralWidget())->count() > 1)
                qobject_cast<QTabWidget*>(centralWidget())->removeTab(1);

            {
                TabControls& tc = GetTabControls("Main");
                QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
                listWidget->clear();
            }

            fileInfo_ = fi;
            currentFileName_ = fileNames[0];
            setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);

            UpdateMain();
            for (const auto& type : yaml::helper::get_user_type_names(fileInfo_))
            {
                QWidget* widgetTabType = CreateTypeTabWidget(QString::fromStdString(type));
                qobject_cast<QTabWidget*>(centralWidget())->addTab(widgetTabType, QString::fromStdString(type));
                UpdateType(QString::fromStdString(type));
            }

            //FillTypeTypeNames();
            FillPropertyTypeNames();
        }
        else
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Ошибка разбора файла %1").arg(fileNames[0]));
        }
    }
}
void MainWindow::on_SaveFile_action()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "parameters_composer",
        QString::fromLocal8Bit("Вы действительно хотите сохранить файл?\nФайл будет перезаписан!"), QMessageBox::No | QMessageBox::Yes);
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

        if (!ReadCurrentFileInfo())
            return;

        std::string message;
        if (!yaml::helper::validate(fileInfo_, message))
        {
            QMessageBox::critical(this, "Validate error", QString::fromLocal8Bit(message.c_str()));
            return;
        }

        if (!yaml::helper::rearrange_types(fileInfo_))
        {
            QMessageBox::critical(this, "Rearrange error", QString::fromLocal8Bit("Возможно, петля в типах"));
            return;
        }

        if (!yaml::writer::write(fileNames[0].toStdString(), fileInfo_))
            return;
        
        //YAML::Emitter emitter;
        //if (!WriteCurrent(emitter))
        //    return;

        //QFile file(fileNames[0]);
        //if (file.open(QIODevice::WriteOnly))
        //    file.write(emitter.c_str());

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

        if (!ReadCurrentFileInfo())
            return;

        std::string message;
        if (!yaml::helper::validate(fileInfo_, message))
        {
            QMessageBox::critical(this, "Validate error", QString::fromLocal8Bit(message.c_str()));
            return;
        }

        if (!yaml::helper::rearrange_types(fileInfo_))
        {
            QMessageBox::critical(this, "Rearrange error", QString::fromLocal8Bit("Возможно, петля в типах"));
            return;
        }

        if (!yaml::writer::write(fileNames[0].toStdString(), fileInfo_))
            return;
        
        //YAML::Emitter emitter;
        //if (!WriteCurrent(emitter))
        //    return;

        //QFile file(fileNames[0]);
        //if (file.open(QIODevice::WriteOnly))
        //    file.write(emitter.c_str());

        currentFileName_ = fileNames[0];
        setWindowTitle("parameters_composer - " + currentFileName_ == "" ? "Untitled" : currentFileName_);
    }
}

void MainWindow::on_AddType_action()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add type", QString::fromLocal8Bit("Имя типа:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        for (const auto& ti : fileInfo_.types)
        {
            if (QString::fromStdString(ti.name) == text)
            {
                QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Тип с именем %1 уже существует").arg(text));
                return;
            }
        }

        QWidget* widgetTabType = CreateTypeTabWidget(text);
        qobject_cast<QTabWidget*>(centralWidget())->addTab(widgetTabType, text);
        qobject_cast<QTabWidget*>(centralWidget())->setCurrentWidget(widgetTabType);

        TabControls& tc = GetTabControls(text);
        qobject_cast<QLineEdit*>(tc.Info["NAME"])->setText(text);

        yaml::type_info ti{};
        ti.name = text.toStdString();
        fileInfo_.types.push_back(ti);

        //FillTypeTypeNames();
        FillPropertyTypeNames();
    }
}

void MainWindow::on_RemoveType_action()
{
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget());
    QString name = tabWidget->tabText(tabWidget->indexOf(tabWidget->currentWidget()));

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
        QMessageBox::StandardButton resBtn = QMessageBox::critical(this, "parameters_composer", message);
        return;
    }

    tabWidget->removeTab(tabWidget->indexOf(tabWidget->currentWidget()));
    auto it = std::find_if(fileInfo_.types.cbegin(), fileInfo_.types.cend(), [name](auto& t) { if (t.name == name.toStdString()) return true; else return false; });
    fileInfo_.types.erase(it);

    //FillTypeTypeNames();
    FillPropertyTypeNames();
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
    connect(listWidget, &QListWidget::currentItemChanged, this, &MainWindow::on_CurrentItemChanged);

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
    connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::on_EditingFinished);
    gridLayout->addWidget(lineEdit, index, 1);

    auto& tc = GetControls(type, group);
    tc[name] = lineEdit;
}

void MainWindow::AddComboBoxPropertyType(QGridLayout* gridLayout, QString name, int index, QString type, MainWindow::ControlsGroup group)
{
    gridLayout->addWidget(new QLabel(name), index, 0);

    QComboBox* comboBox = new QComboBox;
    comboBox->setEditable(false);
    comboBox->setProperty("name", name);
    comboBox->setProperty("group", static_cast<int>(group));
    comboBox->setProperty("type", type);

    for (const auto& s : yaml::helper::get_property_type_names(fileInfo_))
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

    for (const auto& s : yaml::helper::get_type_type_names())
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

bool MainWindow::ReadCurrentParameter(QString type, yaml::parameter_info& pi)
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
    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
    return (listWidget->selectedItems().size() > 0);
}

bool MainWindow::ReadCurrentMainInfo(yaml::info_info& mi)
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

bool MainWindow::ReadCurrentTypeInfo(QString type, yaml::type_info& ti)
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
    yaml::info_info iim{};
    if (!ReadCurrentMainInfo(iim))
        return false;
    fileInfo_.info = iim;

    if (HaveCurrentParameter("Main"))
    {
        yaml::parameter_info pim{};
        if (!ReadCurrentParameter("Main", pim))
            return false;

        if (!yaml::helper::set_parameter_info(fileInfo_, "Main", pim))
            return false;
    }

    for (const auto& type : yaml::helper::get_user_type_names(fileInfo_))
    {
        yaml::type_info tit{};
        if (!ReadCurrentTypeInfo(QString::fromStdString(type), tit))
            return false;

        if (!yaml::helper::set_type_info(fileInfo_, tit.name, tit, true))
            return false;

        if (HaveCurrentParameter(QString::fromStdString(type)))
        {
            yaml::parameter_info pit;
            if (!ReadCurrentParameter(QString::fromStdString(tit.name), pit))
                return false;

            if (!yaml::helper::set_parameter_info(fileInfo_, tit.name, pit))
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
    for (const auto& s : yaml::helper::get_property_type_names(fileInfo_))
        comboBoxMain->addItem(QString::fromStdString(s));
    comboBoxMain->setCurrentText(textMain);

    for (int i = 0; i < tabs_.size(); i++)
    {
        TabControls& tct = tabs_[i];

        QComboBox* comboBoxPropertyType = qobject_cast<QComboBox*>(tct.Properties["TYPE"]);
        QString textPropertyType = comboBoxPropertyType->currentText();
        comboBoxPropertyType->clear();
        for (const auto& s : yaml::helper::get_property_type_names(fileInfo_))
            comboBoxPropertyType->addItem(QString::fromStdString(s));
        comboBoxPropertyType->setCurrentText(textPropertyType);
    }

    return true;
}

//bool MainWindow::FillTypeTypeNames()
//{
//    for (const auto& type : yaml::helper::get_user_type_names(fileInfo_))
//    {
//        TabControls& tct = GetTabControls(QString::fromStdString(type));
//
//        QComboBox* comboBoxTypeType = qobject_cast<QComboBox*>(tct.Info["TYPE"]);
//        QString textTypeType = comboBoxTypeType->currentText();
//        comboBoxTypeType->clear();
//        for (const auto& s : yaml::helper::get_type_type_names())
//            comboBoxTypeType->addItem(QString::fromStdString(s));
//        comboBoxTypeType->setCurrentText(textTypeType);
//    }
//
//    return true;
//}

QWidget* MainWindow::CreatePropertiesWidget(QString type)
{
    QGridLayout* gridLayoutProperties = new QGridLayout;

    int index = 0;
    AddLineEditRequiredProperty(gridLayoutProperties, "NAME", index++, type, MainWindow::ControlsGroup::Properties);
    AddComboBoxPropertyType(gridLayoutProperties, "TYPE", index++, type, MainWindow::ControlsGroup::Properties);
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
    connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);
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
    connect(toolButtonPropertyListRemove, &QToolButton::clicked, this, &MainWindow::on_ListControlClicked);

    QToolButton* toolButtonPropertyListUp = new QToolButton;
    toolButtonPropertyListUp->setFixedSize(buttonSize, buttonSize);
    toolButtonPropertyListUp->setIconSize(QSize(buttonSize, buttonSize));
    toolButtonPropertyListUp->setIcon(QIcon(":/images/up.png"));
    toolButtonPropertyListUp->setProperty("type", type);
    toolButtonPropertyListUp->setProperty("group", static_cast<int>(group));
    toolButtonPropertyListUp->setProperty("name", name);
    toolButtonPropertyListUp->setProperty("action", "up");
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

    if (group == ControlsGroup::PropertyList && name == "PROPERTIES" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add property", QString::fromLocal8Bit("Имя параметра:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (yaml::helper::get_parameter_info(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Параметр с именем %1 уже существует").arg(text));
            return;
        }

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        yaml::parameter_info pi{};
        pi.name = text.toStdString();
        pi.type = "string";
        pi.required = true;
        if (!yaml::helper::add_parameter_info(fileInfo_, type.toStdString(), pi))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
    }
    else if (group == ControlsGroup::PropertyList && name == "PROPERTIES" && action == "remove")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
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
        if (!yaml::helper::remove_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString()))
            return;
    }
    else if (group == ControlsGroup::PropertyList && name == "PROPERTIES" && action == "up")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
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
        if (!yaml::helper::move_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString(), true))
            return;
    }
    else if (group == ControlsGroup::PropertyList && name == "PROPERTIES" && action == "down")
    {
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["PROPERTIES"]);
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
        if (!yaml::helper::move_parameter_info(fileInfo_, type.toStdString(), propertyName.toStdString(), false))
            return;
    }
    else if (group == ControlsGroup::Info && name == "VALUES" && action == "add")
    {
        bool ok;
        QString textName = QInputDialog::getText(this, "Add value", QString::fromLocal8Bit("Имя:"), QLineEdit::Normal, "", &ok);
        if (!ok || textName.isEmpty())
            return;

        // Validate
        if (yaml::helper::have_info_value(fileInfo_, type.toStdString(), textName.toStdString()))
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
        if (!yaml::helper::add_info_value(fileInfo_, type.toStdString(), textName.toStdString(), textValue.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
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
        if (!yaml::helper::remove_info_value(fileInfo_, type.toStdString(), s[0].toStdString()))
            return;
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
        if (!yaml::helper::move_info_value(fileInfo_, type.toStdString(), s[0].toStdString(), true))
            return;
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
        if (!yaml::helper::move_info_value(fileInfo_, type.toStdString(), s[0].toStdString(), false))
            return;
    }
    else if (group == ControlsGroup::Info && name == "INCLUDES" && action == "add")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Add include", QString::fromLocal8Bit("Путь:"), QLineEdit::Normal, "", &ok);
        if (!ok || text.isEmpty())
            return;

        // Validate
        if (yaml::helper::have_info_include(fileInfo_, type.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        auto& tc = GetControls(type, group);
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["INCLUDES"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!yaml::helper::add_info_include(fileInfo_, type.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
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
        if (!yaml::helper::remove_info_include(fileInfo_, type.toStdString(), propertyName.toStdString()))
            return;
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
        if (!yaml::helper::move_info_include(fileInfo_, type.toStdString(), propertyName.toStdString(), true))
            return;
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
        if (!yaml::helper::move_info_include(fileInfo_, type.toStdString(), propertyName.toStdString(), false))
            return;
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
        if (yaml::helper::have_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!yaml::helper::add_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
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
        if (!yaml::helper::remove_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;
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
        if (!yaml::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;
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
        if (!yaml::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;
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
        if (yaml::helper::have_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!yaml::helper::add_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
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
        if (!yaml::helper::remove_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;
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
        if (!yaml::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;
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
        if (!yaml::helper::move_properties_set_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;
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
        if (yaml::helper::have_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), QString("%1").arg(value).toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(value));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["SET_COUNT"]);
        listWidget->addItem(new QListWidgetItem(QString("%1").arg(value)));

        // Add to fileInfo_
        if (!yaml::helper::add_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), QString("%1").arg(value).toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
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
        if (!yaml::helper::remove_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;
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
        if (!yaml::helper::move_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;
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
        if (!yaml::helper::move_properties_set_count_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;
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
        if (yaml::helper::have_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Значение %1 уже существует").arg(text));
            return;
        }

        // Add to control
        QListWidget* listWidget = qobject_cast<QListWidget*>(tc["IDS"]);
        listWidget->addItem(new QListWidgetItem(text));

        // Add to fileInfo_
        if (!yaml::helper::add_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), text.toStdString()))
            return;

        // Update
        listWidget->setCurrentRow(listWidget->count() - 1);
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
        if (!yaml::helper::remove_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString()))
            return;
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
        if (!yaml::helper::move_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), true))
            return;
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
        if (!yaml::helper::move_properties_ids_value(fileInfo_, type.toStdString(), propertyName.toStdString(), value.toStdString(), false))
            return;
    }
}

void MainWindow::on_CurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QListWidget* list = qobject_cast<QListWidget*>(sender());
    QString type = list->property("type").toString();

    yaml::parameter_info pic{};
    if (current != nullptr)
    {
        yaml::parameter_info* ppic = yaml::helper::get_parameter_info(fileInfo_, type.toStdString(), current->text().toStdString());
        if (ppic != nullptr) pic = *ppic;
    }

    if (previous != nullptr)
    {
        yaml::parameter_info pip{};
        if (!ReadCurrentParameter(type, pip))
            return;

        if (!yaml::helper::set_parameter_info(fileInfo_, type.toStdString(), pip))
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
        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget());

        QString oldName = type;
        QString newName = lineEdit->text();

        if (yaml::helper::get_type_info(fileInfo_, newName.toStdString()))
        {
            QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Тип с именем %1 уже существует").arg(newName));
            lineEdit->setText(oldName);
            return;
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

        // Update fileInfo_
        for (auto& t : fileInfo_.types)
        {
            if (QString::fromStdString(t.name) == type)
            {
                t.name = newName.toStdString();
                break;
            }
        }

        for (auto& p : fileInfo_.parameters)
        {
            if (QString::fromStdString(p.type) == oldName)
            {
                p.type = newName.toStdString();
                break;
            }
            if (QString::fromStdString(p.type) == arrayOldName)
            {
                p.type = arrayNewName.toStdString();
                break;
            }
        }
        for (auto& t : fileInfo_.types)
        {
            for (auto& p : t.parameters)
            {
                if (QString::fromStdString(p.type) == oldName)
                {
                    p.type = newName.toStdString();
                    break;
                }
                if (QString::fromStdString(p.type) == arrayOldName)
                {
                    p.type = arrayNewName.toStdString();
                    break;
                }
            }
            break;
        }

        FillPropertyTypeNames();

        TabControls& tc_m = GetTabControls("Main");
        QComboBox* t_m = qobject_cast<QComboBox*>(tc_m.Properties["TYPE"]);
        const auto& pi_m = yaml::helper::get_parameter_info(fileInfo_, "Main", "TYPE");
        t_m->setCurrentText(QString::fromStdString(pi_m->type));

        for (auto& t : fileInfo_.types)
        {
            TabControls& tc_t = GetTabControls(QString::fromStdString(t.name));
            QComboBox* t_t = qobject_cast<QComboBox*>(tc_m.Properties["TYPE"]);
            const auto& pi_t = yaml::helper::get_parameter_info(fileInfo_, t.name, "TYPE");
            t_t->setCurrentText(QString::fromStdString(pi_m->type));
        }

        //TabControls& tc_m = GetTabControls("Main");
        //QComboBox* t_m = qobject_cast<QComboBox*>(tc_m.Properties["TYPE"]);
        //if (t_m->currentText() == oldName)
        //{
        //    t_m->setCurrentText(newName);
        //}
        //if (t_m->currentText() == arrayOldName)
        //{
        //    t_m->setCurrentText(arrayNewName);
        //}

        //for (auto& t : fileInfo_.types)
        //{
        //    TabControls& tc_t = GetTabControls(QString::fromStdString(t.name));
        //    QComboBox* t_t = qobject_cast<QComboBox*>(tc_m.Properties["TYPE"]);
        //    if (t_t->currentText() == oldName)
        //    {
        //        t_t->setCurrentText(newName);
        //    }
        //    if (t_t->currentText() == arrayOldName)
        //    {
        //        t_t->setCurrentText(arrayNewName);
        //    }
        //}

        RenameTabControls(oldName, newName);
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
                QMessageBox::critical(this, "Error", QString::fromLocal8Bit("Свойство с именем %1 уже существует").arg(newName));
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
                if (QString::fromStdString(p.name) == oldName)
                {
                    p.name = newName.toStdString();
                    break;
                }
            }
        }
        else
        {
            for (auto& t : fileInfo_.types)
            {
                if (QString::fromStdString(t.name) == type)
                {
                    for (auto& p : t.parameters)
                    {
                        if (QString::fromStdString(p.name) == oldName)
                        {
                            p.name = newName.toStdString();
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

    yaml::type_info tit{};
    if (!ReadCurrentTypeInfo(type, tit))
        return;

    auto ti = yaml::helper::get_type_info(fileInfo_, type.toStdString());
    ti->type = tit.type;
    //if (!yaml::helper::set_type_info(fileInfo_, type.toStdString(), tit, true))
    //    return;

    FillPropertyTypeNames();
}

void MainWindow::Update()
{
    UpdateMain();
    for (const auto& type : yaml::helper::get_user_type_names(fileInfo_))
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

    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
    for (const auto& pi : fileInfo_.parameters)
        listWidget->addItem(QString::fromStdString(pi.name));
    if (listWidget->count() > 0)
        listWidget->setCurrentRow(0);
}

void MainWindow::UpdateType(QString type)
{
    TabControls& tc = GetTabControls(type);
    const auto& ti = yaml::helper::get_type_info(fileInfo_, type.toStdString());

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

    QListWidget* listWidget = qobject_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
    listWidget->clear();
    for (const auto& pi : ti->parameters)
        listWidget->addItem(QString::fromStdString(pi.name));
    if (listWidget->count() > 0)
        listWidget->setCurrentRow(0);
}
