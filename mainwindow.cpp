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

#include "mainwindow.h"
//#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
//    , ui(new Ui::MainWindow)
{
    CreateUi();

    //ui->setupUi(this);
    setWindowIcon(QIcon(":/images/parameters.png"));




//    ui->toolButtonAddProperty->setFixedSize(32, 32);
//    ui->toolButtonAddProperty->setIconSize(QSize(32, 32));
//    ui->toolButtonAddProperty->setIcon(QIcon(":/images/plus.png"));

//    ui->toolButtonRemoveProperty->setFixedSize(32, 32);
//    ui->toolButtonRemoveProperty->setIconSize(QSize(32, 32));
//    ui->toolButtonRemoveProperty->setIcon(QIcon(":/images/minus.png"));

//    ui->toolButtonUpProperty->setFixedSize(32, 32);
//    ui->toolButtonUpProperty->setIconSize(QSize(32, 32));
//    ui->toolButtonUpProperty->setIcon(QIcon(":/images/up.png"));

//    ui->toolButtonDownProperty->setFixedSize(32, 32);
//    ui->toolButtonDownProperty->setIconSize(QSize(32, 32));
//    ui->toolButtonDownProperty->setIcon(QIcon(":/images/down.png"));




//    ui->toolButtonPropertyRestrictionSetAdd->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionSetAdd->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionSetAdd->setIcon(QIcon(":/images/plus.png"));

//    ui->toolButtonPropertyRestrictionSetRemove->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionSetRemove->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionSetRemove->setIcon(QIcon(":/images/minus.png"));

//    ui->toolButtonPropertyRestrictionSetUp->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionSetUp->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionSetUp->setIcon(QIcon(":/images/up.png"));

//    ui->toolButtonPropertyRestrictionSetDown->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionSetDown->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionSetDown->setIcon(QIcon(":/images/down.png"));





//    ui->toolButtonPropertyRestrictionCountSetAdd->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionCountSetAdd->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionCountSetAdd->setIcon(QIcon(":/images/plus.png"));

//    ui->toolButtonPropertyRestrictionCountSetRemove->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionCountSetRemove->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionCountSetRemove->setIcon(QIcon(":/images/minus.png"));

//    ui->toolButtonPropertyRestrictionCountSetUp->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionCountSetUp->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionCountSetUp->setIcon(QIcon(":/images/up.png"));

//    ui->toolButtonPropertyRestrictionCountSetDown->setFixedSize(24, 24);
//    ui->toolButtonPropertyRestrictionCountSetDown->setIconSize(QSize(24, 24));
//    ui->toolButtonPropertyRestrictionCountSetDown->setIcon(QIcon(":/images/down.png"));





//    ui->splitter->setStretchFactor(0, 0);
//    ui->splitter->setStretchFactor(1, 1);

//    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);
}

MainWindow::~MainWindow()
{
//    delete ui;
}

void MainWindow::CreateUi()
{
    resize(1000, 600);
    QTabWidget* tabWidget = new QTabWidget;
    QWidget* widgetTabProperties = CreatePropertiesTabWidget();
    tabWidget->addTab(widgetTabProperties, "Properties");
    QWidget* tab2 = new QWidget(tabWidget);
    tabWidget->addTab(tab2, "Properties2");

    setCentralWidget(tabWidget);
}

QWidget* MainWindow::CreatePropertiesTabWidget()
{
    QWidget* widgetTabProperties = new QWidget;

    QWidget* widgetSplitterInfo = CreatePropertiesTabInfoWidget();
    QWidget* widgetSplitterPropertyList = CreatePropertiesTabPropertyListWidget();
    QWidget* widgetSplitterProperties = CreatePropertiesTabPropertiesWidget();

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

QWidget* MainWindow::CreatePropertiesTabInfoWidget()
{
    QGridLayout* gridLayoutInfo = new QGridLayout;
 
    int index = 0;
    AddPropertySubheader(gridLayoutInfo, "INFO", "font-weight: bold; font-size: 14px", index++);
    AddLineEditProperty(gridLayoutInfo, "ID", index++);
    AddLineEditProperty(gridLayoutInfo, "DISPLAY_NAME", index++);
    AddPlainTextEditProperty(gridLayoutInfo, "DESCRIPTION", index++);
    AddLineEditProperty(gridLayoutInfo, "CATEGORY", index++);
    AddLineEditProperty(gridLayoutInfo, "PICTOGRAM", index++);
    AddLineEditProperty(gridLayoutInfo, "HINT", index++);
    AddLineEditProperty(gridLayoutInfo, "AUTHOR", index++);
    AddLineEditProperty(gridLayoutInfo, "WIKI", index++);

    //QLabel* labelInfoHeader = new QLabel;
    //labelInfoHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    //labelInfoHeader->setText("INFO");
    //gridLayoutInfo->addWidget(labelInfoHeader, 0, 0, 1, 2, Qt::AlignCenter);

    //gridLayoutInfo->addWidget(new QLabel("ID"), 1, 0);
    //QLineEdit* lineEditInfoId = new QLineEdit;
    //lineEditInfoId->setObjectName("lineEditInfoId");
    //gridLayoutInfo->addWidget(lineEditInfoId, 1, 1);

    //gridLayoutInfo->addWidget(new QLabel("DISPLAY_NAME"), 2, 0);
    //QLineEdit* lineEditInfoDisplayName = new QLineEdit;
    //lineEditInfoDisplayName->setObjectName("lineEditInfoDisplayName");
    //gridLayoutInfo->addWidget(lineEditInfoDisplayName, 2, 1);

    //gridLayoutInfo->addWidget(new QLabel("DESCRIPTION"), 3, 0);
    //QPlainTextEdit* lineEditInfoDescription = new QPlainTextEdit;
    //lineEditInfoDescription->setObjectName("lineEditInfoDescription");
    //gridLayoutInfo->addWidget(lineEditInfoDescription, 3, 1);

    //gridLayoutInfo->addWidget(new QLabel("CATEGORY"), 4, 0);
    //QLineEdit* lineEditInfoCategory = new QLineEdit;
    //lineEditInfoCategory->setObjectName("lineEditInfoCategory");
    //gridLayoutInfo->addWidget(lineEditInfoCategory, 4, 1);

    //gridLayoutInfo->addWidget(new QLabel("PICTOGRAM"), 5, 0);
    //QLineEdit* lineEditInfoPictogram = new QLineEdit;
    //lineEditInfoPictogram->setObjectName("lineEditInfoPictogram");
    //gridLayoutInfo->addWidget(lineEditInfoPictogram, 5, 1);

    //gridLayoutInfo->addWidget(new QLabel("HINT"), 6, 0);
    //QLineEdit* lineEditInfoHint = new QLineEdit;
    //lineEditInfoHint->setObjectName("lineEditInfoHint");
    //gridLayoutInfo->addWidget(lineEditInfoHint, 6, 1);

    //gridLayoutInfo->addWidget(new QLabel("AUTHOR"), 7, 0);
    //QLineEdit* lineEditInfoAuthor = new QLineEdit;
    //lineEditInfoAuthor->setObjectName("lineEditInfoAuthor");
    //gridLayoutInfo->addWidget(lineEditInfoAuthor, 7, 1);

    //gridLayoutInfo->addWidget(new QLabel("WIKI"), 8, 0);
    //QLineEdit* lineEditInfoWiki = new QLineEdit;
    //lineEditInfoWiki->setObjectName("lineEditInfoWiki");
    //gridLayoutInfo->addWidget(lineEditInfoWiki, 8, 1);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

QWidget* MainWindow::CreatePropertiesTabPropertyListWidget()
{
    QLabel* labelPropertyListHeader = new QLabel;
    labelPropertyListHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelPropertyListHeader->setText("PROPERITES");

    QWidget* widgetPropertyListButtons = CreateToolBoxListControlWidget(32, "PropertiesTab", "PropertyList");

    QVBoxLayout* vBoxLayoutPropertyList = new QVBoxLayout;
    vBoxLayoutPropertyList->addWidget(labelPropertyListHeader, 0, Qt::AlignCenter);
    vBoxLayoutPropertyList->addWidget(widgetPropertyListButtons);
    vBoxLayoutPropertyList->addWidget(new QListWidget, 1);
    vBoxLayoutPropertyList->addStretch();

    QWidget* widgetSplitterPropertyList = new QWidget;
    widgetSplitterPropertyList->setLayout(vBoxLayoutPropertyList);

    return widgetSplitterPropertyList;
}

void MainWindow::AddLineEditProperty(QGridLayout* gridLayout, QString name, int index)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QLineEdit* lineEdit = new QLineEdit;
    //lineEdit->setObjectName("lineEditPropertiesName");
    gridLayout->addWidget(lineEdit, index, 1);
}

void MainWindow::AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QPlainTextEdit* plainTextEdit = new QPlainTextEdit;
    //plainTextEdit->setObjectName("plainTextEditPropertiesName");
    gridLayout->addWidget(plainTextEdit, index, 1);
}

void MainWindow::AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index)
{
    gridLayout->addWidget(new QLabel(name), index, 0);
    QCheckBox* checkBox = new QCheckBox;
    //checkBox->setObjectName("checkBoxPropertiesName");
    gridLayout->addWidget(checkBox, index, 1);
}

void MainWindow::AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index)
{
    QLabel* label = new QLabel;
    label->setStyleSheet(style);
    label->setText(text);
    gridLayout->addWidget(label, index, 0, 1, 2, Qt::AlignCenter);
}

void MainWindow::AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId)
{
    QWidget* widgetPropertiesRestrictionsSetButtons = CreateToolBoxListControlWidget(24, tabId, listControlId);
    QVBoxLayout* vBoxLayoutPropertiesRestrictionsSet = new QVBoxLayout;
    vBoxLayoutPropertiesRestrictionsSet->addWidget(widgetPropertiesRestrictionsSetButtons);
    vBoxLayoutPropertiesRestrictionsSet->addWidget(new QListWidget, 1);
    vBoxLayoutPropertiesRestrictionsSet->addStretch();
    vBoxLayoutPropertiesRestrictionsSet->setMargin(0);
    QWidget* widgetPropertiesRestrictionsSet = new QWidget;
    widgetPropertiesRestrictionsSet->setLayout(vBoxLayoutPropertiesRestrictionsSet);
    gridLayout->addWidget(new QLabel(name), index, 0);
    gridLayout->addWidget(widgetPropertiesRestrictionsSet, index, 1);
}

QWidget* MainWindow::CreatePropertiesTabPropertiesWidget()
{
    QGridLayout* gridLayoutProperties = new QGridLayout;

    int index = 0;
    AddLineEditProperty(gridLayoutProperties, "NAME", index++);
    AddLineEditProperty(gridLayoutProperties, "TYPE", index++);
    AddLineEditProperty(gridLayoutProperties, "DISPLAY_NAME", index++);
    AddPlainTextEditProperty(gridLayoutProperties, "DESCRIPTION", index++);
    AddCheckBoxProperty(gridLayoutProperties, "REQUIRED", index++);
    AddLineEditProperty(gridLayoutProperties, "DEFAULT", index++);
    AddLineEditProperty(gridLayoutProperties, "HINT", index++);
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (base)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN", index++);
    AddLineEditProperty(gridLayoutProperties, "MAX", index++);
    AddListProperty(gridLayoutProperties, "SET", index++, "PropertiesTab", "RestrictionsSet");
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (array)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MIN_COUNT", index++);
    AddLineEditProperty(gridLayoutProperties, "MAX_COUNT", index++);
    AddListProperty(gridLayoutProperties, "SET_COUNT", index++, "PropertiesTab", "RestrictionsSetCount");
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (unit)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "CATEGORY", index++);
    AddListProperty(gridLayoutProperties, "IDS", index++, "PropertiesTab", "RestrictionsIds");
    AddPropertySubheader(gridLayoutProperties, "RESTRICTIONS (path)", "font-size: 14px", index++);
    AddLineEditProperty(gridLayoutProperties, "MAX_LENGTH", index++);


    //gridLayoutProperties->addWidget(new QLabel("NAME"), 0, 0);
    //QLineEdit* lineEditPropertiesName = new QLineEdit;
    //lineEditPropertiesName->setObjectName("lineEditPropertiesName");
    //gridLayoutProperties->addWidget(lineEditPropertiesName, 0, 1);

    //gridLayoutProperties->addWidget(new QLabel("TYPE"), 1, 0);
    //QLineEdit* lineEditPropertiesType = new QLineEdit;
    //lineEditPropertiesType->setObjectName("lineEditPropertiesType");
    //gridLayoutProperties->addWidget(lineEditPropertiesType, 1, 1);

    //gridLayoutProperties->addWidget(new QLabel("DISPLAY_NAME"), 2, 0);
    //QLineEdit* lineEditPropertiesDisplayName = new QLineEdit;
    //lineEditPropertiesDisplayName->setObjectName("lineEditPropertiesDisplayName");
    //gridLayoutProperties->addWidget(lineEditPropertiesDisplayName, 2, 1);

    //gridLayoutProperties->addWidget(new QLabel("DESCRIPTION"), 3, 0);
    //QPlainTextEdit* lineEditPropertiesDescription = new QPlainTextEdit;
    //lineEditPropertiesDescription->setObjectName("lineEditPropertiesDescription");
    //gridLayoutProperties->addWidget(lineEditPropertiesDescription, 3, 1);

    //gridLayoutProperties->addWidget(new QLabel("REQUIRED"), 4, 0);
    //QCheckBox* checkBoxPropertiesRequired = new QCheckBox;
    //checkBoxPropertiesRequired->setObjectName("checkBoxPropertiesRequired");
    //gridLayoutProperties->addWidget(checkBoxPropertiesRequired, 4, 1);

    //gridLayoutProperties->addWidget(new QLabel("DEFAULT"), 5, 0);
    //QLineEdit* lineEditPropertiesDefault = new QLineEdit;
    //lineEditPropertiesDefault->setObjectName("lineEditPropertiesDefault");
    //gridLayoutProperties->addWidget(lineEditPropertiesDefault, 5, 1);

    //gridLayoutProperties->addWidget(new QLabel("HINT"), 6, 0);
    //QLineEdit* lineEditPropertiesHint = new QLineEdit;
    //lineEditPropertiesHint->setObjectName("lineEditPropertiesHint");
    //gridLayoutProperties->addWidget(lineEditPropertiesHint, 6, 1);

    //QLabel* labelPropertiesHeader = new QLabel;
    //labelPropertiesHeader->setStyleSheet("font-size: 14px");
    //labelPropertiesHeader->setText("RESTRICTIONS (base)");
    //gridLayoutProperties->addWidget(labelPropertiesHeader, 7, 0, 1, 2, Qt::AlignCenter);

    //gridLayoutProperties->addWidget(new QLabel("MIN"), 8, 0);
    //QLineEdit* lineEditPropertiesRestrictionsMin = new QLineEdit;
    //lineEditPropertiesRestrictionsMin->setObjectName("lineEditPropertiesRestrictionsMin");
    //gridLayoutProperties->addWidget(lineEditPropertiesRestrictionsMin, 8, 1);

    //gridLayoutProperties->addWidget(new QLabel("MAX"), 9, 0);
    //QLineEdit* lineEditPropertiesRestrictionsMax = new QLineEdit;
    //lineEditPropertiesRestrictionsMax->setObjectName("lineEditPropertiesRestrictionsMax");
    //gridLayoutProperties->addWidget(lineEditPropertiesRestrictionsMax, 9, 1);

    //QWidget* widgetPropertiesRestrictionsSetButtons = CreateToolBoxListControlWidget(24, "PropertiesTab", "RestrictionsSet");
    //QVBoxLayout* vBoxLayoutPropertiesRestrictionsSet = new QVBoxLayout;
    //vBoxLayoutPropertiesRestrictionsSet->addWidget(widgetPropertiesRestrictionsSetButtons);
    //vBoxLayoutPropertiesRestrictionsSet->addWidget(new QListWidget, 1);
    //vBoxLayoutPropertiesRestrictionsSet->addStretch();
    //vBoxLayoutPropertiesRestrictionsSet->setMargin(0);
    //QWidget* widgetPropertiesRestrictionsSet = new QWidget;
    //widgetPropertiesRestrictionsSet->setLayout(vBoxLayoutPropertiesRestrictionsSet);
    //gridLayoutProperties->addWidget(new QLabel("SET"), 10, 0);
    //gridLayoutProperties->addWidget(widgetPropertiesRestrictionsSet, 10, 1);

    //QLabel* labelPropertiesRestrictionUnitHeader = new QLabel;
    //labelPropertiesRestrictionUnitHeader->setStyleSheet("font-size: 14px");
    //labelPropertiesRestrictionUnitHeader->setText("RESTRICTIONS (unit)");
    //gridLayoutProperties->addWidget(labelPropertiesRestrictionUnitHeader, 11, 0, 1, 2, Qt::AlignCenter);

    //gridLayoutProperties->addWidget(new QLabel("MIN_COUNT"), 11, 0);
    //QLineEdit* lineEditPropertiesRestrictionsMinCount = new QLineEdit;
    //lineEditPropertiesRestrictionsMinCount->setObjectName("lineEditPropertiesRestrictionsMinCount");
    //gridLayoutProperties->addWidget(lineEditPropertiesRestrictionsMinCount, 11, 1);

    //gridLayoutProperties->addWidget(new QLabel("MAX_COUNT"), 12, 0);
    //QLineEdit* lineEditPropertiesRestrictionsMaxCount = new QLineEdit;
    //lineEditPropertiesRestrictionsMaxCount->setObjectName("lineEditPropertiesRestrictionsMaxCount");
    //gridLayoutProperties->addWidget(lineEditPropertiesRestrictionsMaxCount, 12, 1);

    //QWidget* widgetPropertiesRestrictionsSetCountButtons = CreateToolBoxListControlWidget(24, "PropertiesTab", "RestrictionsSetCount");
    //QVBoxLayout* vBoxLayoutPropertiesRestrictionsSetCount = new QVBoxLayout;
    //vBoxLayoutPropertiesRestrictionsSetCount->addWidget(widgetPropertiesRestrictionsSetCountButtons);
    //vBoxLayoutPropertiesRestrictionsSetCount->addWidget(new QListWidget, 1);
    //vBoxLayoutPropertiesRestrictionsSetCount->addStretch();
    //vBoxLayoutPropertiesRestrictionsSetCount->setMargin(0);
    //QWidget* widgetPropertiesRestrictionsSetCount = new QWidget;
    //widgetPropertiesRestrictionsSetCount->setLayout(vBoxLayoutPropertiesRestrictionsSetCount);
    //gridLayoutProperties->addWidget(new QLabel("SET_COUNT"), 13, 0);
    //gridLayoutProperties->addWidget(widgetPropertiesRestrictionsSetCount, 13, 1);





    gridLayoutProperties->setRowStretch(gridLayoutProperties->rowCount(), 1);

    QWidget* widgetSplitterProperties = new QWidget;
    widgetSplitterProperties->setLayout(gridLayoutProperties);

    QScrollArea* scrollAreaProperties = new QScrollArea;
    scrollAreaProperties->setWidget(widgetSplitterProperties);
    scrollAreaProperties->setWidgetResizable(true);
    scrollAreaProperties->setFrameStyle(0);

    return scrollAreaProperties;
}

QWidget* MainWindow::CreateToolBoxListControlWidget(int buttonSize, QString tabId, QString listControlId)
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

QWidget* MainWindow::CreateYmlTypeTabWidget()
{
    QWidget* widgetTabProperties = new QWidget;

    QWidget* widgetSplitterInfo = CreatePropertiesTabInfoWidget();
    QWidget* widgetSplitterPropertyList = CreatePropertiesTabPropertyListWidget();
    QWidget* widgetSplitterProperties = CreatePropertiesTabPropertiesWidget();

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

QWidget* MainWindow::CreateYmlTypeTabInfoWidget()
{

}

QWidget* MainWindow::CreateCppTypeTabWidget()
{

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
    if(current != 0)
        qDebug() << current->text();

}


void MainWindow::on_toolButtonRemoveProperty_clicked()
{
//    auto si = ui->listWidgetProperties->selectedItems();
//    if (si.size() > 0)
//    {
//        delete si[0];
//    }
}

