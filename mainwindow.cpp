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

#include "yaml_parser.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    currentFileName_ = "Untitled";
    parser_ = nullptr;

    CreateUi();

    setWindowIcon(QIcon(":/images/parameters.png"));
    setWindowTitle("parameters_composer - " + currentFileName_);
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_NewFile_action()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Test", "Quit?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qDebug() << "Yes was clicked";
        QApplication::quit();
    }
    else {
        qDebug() << "Yes was *not* clicked";
    }
}

void MainWindow::on_Quit_action()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Test", "Quit?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qDebug() << "Yes was clicked";
        QApplication::quit();
    }
    else {
        qDebug() << "Yes was *not* clicked";
    }
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
        if (parser_ != nullptr)
            delete parser_;
        parser_ = new yaml::yaml_parser(false);
        if (parser_->parse(fileNames[0].toStdString()))
        {
            currentFileName_ = fileNames[0];
            setWindowTitle("parameters_composer - " + currentFileName_);
            yaml::file_info fi = parser_->get_file_info();

            {
                QString typeName("Main");
                if (!tabs_.contains(typeName))
                    tabs_[typeName] = {};
                TabControls& tc = tabs_[typeName];

                dynamic_cast<QLineEdit*>(tc.Info["ID"])->setText(QString(fi.info.yml.id.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["DISPLAY_NAME"])->setText(QString(fi.info.yml.display_name.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["CATEGORY"])->setText(QString(fi.info.yml.category.c_str()));
                dynamic_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString(fi.info.yml.description.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["PICTOGRAM"])->setText(QString(fi.info.yml.pictogram.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["HINT"])->setText(QString(fi.info.yml.hint.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["AUTHOR"])->setText(QString(fi.info.yml.author.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["WIKI"])->setText(QString(fi.info.yml.wiki.c_str()));

                QListWidget* listWidget = dynamic_cast<QListWidget*>(tc.PropertyList["PROPERTIES"]);
                listWidget->clear();
                for (const auto& pi : fi.parameters)
                    listWidget->addItem(QString(pi.yml.name.c_str()));
                if (listWidget->count() > 0)
                    listWidget->setCurrentRow(0);
            }

            for (const auto& ti : fi.types)
            {
                QString typeName(ti.yml.name.c_str());

                QWidget* widgetTabType = CreateTypeTabWidget(typeName);
                dynamic_cast<QTabWidget*>(centralWidget())->addTab(widgetTabType, typeName);

                if (!tabs_.contains(typeName))
                    tabs_[typeName] = {};
                TabControls& tc = tabs_[typeName];

                dynamic_cast<QLineEdit*>(tc.Info["NAME"])->setText(QString(ti.yml.name.c_str()));
                dynamic_cast<QLineEdit*>(tc.Info["TYPE"])->setText(QString(ti.yml.type.c_str()));
                dynamic_cast<QPlainTextEdit*>(tc.Info["DESCRIPTION"])->setPlainText(QString(ti.yml.description.c_str()));

                QListWidget* listWidgetValues = dynamic_cast<QListWidget*>(tc.Info["VALUES"]);
                listWidgetValues->clear();
                for (const auto& v : ti.yml.values)
                    listWidgetValues->addItem(QString(v.first.c_str()) + " : " + QString(v.second.c_str()));

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

void MainWindow::CreateMenu()
{
    QAction* newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::on_NewFile_action);

    QAction* openAct = new QAction(tr("&Open"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open a file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::on_OpenFile_action);

    QAction* quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit application"));
    connect(quitAct, &QAction::triggered, this, &MainWindow::on_Quit_action);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
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
 
    QString typeName("Main");
    if (!tabs_.contains(typeName))
        tabs_[typeName] = {};
    TabControls& tc = tabs_[typeName];

    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "INFO", "font-weight: bold; font-size: 14px", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "ID", index++, tc.Info);
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

QWidget* MainWindow::CreatePropertyListWidget(QString typeName)
{
    QLabel* labelPropertyListHeader = new QLabel;
    labelPropertyListHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelPropertyListHeader->setText("PROPERITES");

    QWidget* widgetPropertyListButtons = CreateListControlWidget(32, "PropertiesTab", "PropertyList", typeName);

    QListWidget* listWidget = new QListWidget;
    listWidget->setProperty("typeName", typeName);
    connect(listWidget, &QListWidget::currentItemChanged, this, &MainWindow::on_listWidgetProperties_currentItemChanged);

    QVBoxLayout* vBoxLayoutPropertyList = new QVBoxLayout;
    vBoxLayoutPropertyList->addWidget(labelPropertyListHeader, 0, Qt::AlignCenter);
    vBoxLayoutPropertyList->addWidget(widgetPropertyListButtons);
    vBoxLayoutPropertyList->addWidget(listWidget, 1);
    vBoxLayoutPropertyList->addStretch();

    QWidget* widgetSplitterPropertyList = new QWidget;
    widgetSplitterPropertyList->setLayout(vBoxLayoutPropertyList);

    TabControls& tc = tabs_[typeName];
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

void MainWindow::AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index, QMap<QString, QObject*>& mapControls)
{
    QLabel* label = new QLabel;
    label->setStyleSheet(style);
    label->setText(text);
    gridLayout->addWidget(label, index, 0, 1, 2, Qt::AlignCenter);
}

void MainWindow::AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId, QMap<QString, QObject*>& mapControls, QString typeName)
{
    QWidget* widgetPropertiesRestrictionsSetButtons = CreateListControlWidget(24, tabId, listControlId, typeName);
    QListWidget* listWidget = new QListWidget;
    //listWidget->setProperty("typeName", typeName);
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

QWidget* MainWindow::CreatePropertiesWidget(QString typeName)
{
    QGridLayout* gridLayoutProperties = new QGridLayout;

    if (!tabs_.contains(typeName))
        tabs_[typeName] = {};
    TabControls& tc = tabs_[typeName];

    int index = 0;
    AddLineEditProperty(gridLayoutProperties, "NAME", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "TYPE", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "DISPLAY_NAME", index++, tc.Properties);
    AddPlainTextEditProperty(gridLayoutProperties, "DESCRIPTION", index++, tc.Properties);
    AddCheckBoxProperty(gridLayoutProperties, "REQUIRED", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "DEFAULT", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "HINT", index++, tc.Properties);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (base)", "font-size: 14px", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "MIN", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "MAX", index++, tc.Properties);
    AddListProperty(gridLayoutProperties, "SET", index++, "PropertiesTab", "RestrictionsSet", tc.Properties, typeName);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (array)", "font-size: 14px", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "MIN_COUNT", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "MAX_COUNT", index++, tc.Properties);
    AddListProperty(gridLayoutProperties, "SET_COUNT", index++, "PropertiesTab", "RestrictionsSetCount", tc.Properties, typeName);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (unit)", "font-size: 14px", index++, tc.Properties);
    AddLineEditProperty(gridLayoutProperties, "CATEGORY", index++, tc.Properties);
    AddListProperty(gridLayoutProperties, "IDS", index++, "PropertiesTab", "RestrictionsIds", tc.Properties, typeName);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (path)", "font-size: 14px", index++, tc.Properties);
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

QWidget* MainWindow::CreateListControlWidget(int buttonSize, QString tabId, QString listControlId, QString typeName)
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

QWidget* MainWindow::CreateTypeTabWidget(QString typeName)
{
    QWidget* widgetTabProperties = new QWidget;

    QWidget* widgetSplitterInfo = CreateTypeTabInfoWidget(typeName);
    QWidget* widgetSplitterPropertyList = CreatePropertyListWidget(typeName);
    QWidget* widgetSplitterProperties = CreatePropertiesWidget(typeName);

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

QWidget* MainWindow::CreateTypeTabInfoWidget(QString typeName)
{
    QGridLayout* gridLayoutInfo = new QGridLayout;

    if (!tabs_.contains(typeName))
        tabs_[typeName] = {};
    TabControls& tc = tabs_[typeName];

    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "TYPES", "font-weight: bold; font-size: 14px", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "NAME", index++, tc.Info);
    AddLineEditProperty(gridLayoutInfo, "TYPE", index++, tc.Info);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++, tc.Info);
    AddListProperty(gridLayoutInfo, "VALUES", index++, typeName, "Values", tc.Info, typeName);
    AddListProperty(gridLayoutInfo, "INCLUDES", index++, typeName, "Includes", tc.Info, typeName);

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

    QString typeName = list->property("typeName").toString();

    yaml::file_info fi = parser_->get_file_info();
    yaml::parameter_info pi{};
    if (current != nullptr)
    {
        std::vector<yaml::parameter_info>& pis = fi.parameters;
        if (typeName != "Main")
        {
            for (const auto& t : fi.types)
                if (QString(t.yml.name.c_str()) == typeName)
                {
                    pis = t.parameters;
                    break;
                }
        }

        for (const auto& p : pis)
            if (QString(p.yml.name.c_str()) == current->text())
                pi = p;
    }

    if (!tabs_.contains(typeName))
        tabs_[typeName] = {};
    TabControls& tc = tabs_[typeName];

    dynamic_cast<QLineEdit*>(tc.Properties["NAME"])->setText(QString(pi.yml.name.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["TYPE"])->setText(QString(pi.yml.type.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["DISPLAY_NAME"])->setText(QString(pi.yml.display_name.c_str()));
    dynamic_cast<QPlainTextEdit*>(tc.Properties["DESCRIPTION"])->setPlainText(QString(pi.yml.description.c_str()));
    dynamic_cast<QCheckBox*>(tc.Properties["REQUIRED"])->setChecked(pi.is_required);
    dynamic_cast<QLineEdit*>(tc.Properties["DEFAULT"])->setText(QString(pi.yml.default_.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["HINT"])->setText(QString(pi.yml.hint.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["MIN"])->setText(QString(pi.yml.restrictions.min.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["MAX"])->setText(QString(pi.yml.restrictions.max.c_str()));
    QListWidget* listWidgetSet = dynamic_cast<QListWidget*>(tc.Properties["SET"]);
    listWidgetSet->clear();
    for (const auto& s : pi.yml.restrictions.set_)
        listWidgetSet->addItem(QString(s.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["MIN_COUNT"])->setText(QString(pi.yml.restrictions.min_count.c_str()));
    dynamic_cast<QLineEdit*>(tc.Properties["MAX_COUNT"])->setText(QString(pi.yml.restrictions.max_count.c_str()));
    QListWidget* listWidgetSetCount = dynamic_cast<QListWidget*>(tc.Properties["SET_COUNT"]);
    listWidgetSetCount->clear();
    for (const auto& s : pi.yml.restrictions.set_count)
        listWidgetSetCount->addItem(QString(s.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["CATEGORY"])->setText(QString(pi.yml.restrictions.category.c_str()));
    QListWidget* listWidgetIds = dynamic_cast<QListWidget*>(tc.Properties["IDS"]);
    listWidgetIds->clear();
    for (const auto& s : pi.yml.restrictions.ids)
        listWidgetIds->addItem(QString(s.c_str()));

    dynamic_cast<QLineEdit*>(tc.Properties["MAX_LENGTH"])->setText(QString(pi.yml.restrictions.max_length.c_str()));
}


void MainWindow::on_toolButtonRemoveProperty_clicked()
{
//    auto si = ui->listWidgetProperties->selectedItems();
//    if (si.size() > 0)
//    {
//        delete si[0];
//    }
}

