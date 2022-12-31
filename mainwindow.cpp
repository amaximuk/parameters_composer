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
    QLabel* labelInfoHeader = new QLabel;
    labelInfoHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelInfoHeader->setText("INFO");
    gridLayoutInfo->addWidget(labelInfoHeader, 0, 0, 1, 2, Qt::AlignCenter);

    gridLayoutInfo->addWidget(new QLabel("ID"), 1, 0);
    QLineEdit* lineEditInfoId = new QLineEdit;
    lineEditInfoId->setObjectName("lineEditInfoId");
    gridLayoutInfo->addWidget(lineEditInfoId, 1, 1);

    gridLayoutInfo->addWidget(new QLabel("DISPLAY_NAME"), 2, 0);
    QLineEdit* lineEditInfoDisplayName = new QLineEdit;
    lineEditInfoDisplayName->setObjectName("lineEditInfoDisplayName");
    gridLayoutInfo->addWidget(lineEditInfoDisplayName, 2, 1);

    gridLayoutInfo->addWidget(new QLabel("DESCRIPTION"), 3, 0);
    QPlainTextEdit* lineEditInfoDescription = new QPlainTextEdit;
    lineEditInfoDescription->setObjectName("lineEditInfoDescription");
    gridLayoutInfo->addWidget(lineEditInfoDescription, 3, 1);

    gridLayoutInfo->addWidget(new QLabel("CATEGORY"), 4, 0);
    QLineEdit* lineEditInfoCategory = new QLineEdit;
    lineEditInfoCategory->setObjectName("lineEditInfoCategory");
    gridLayoutInfo->addWidget(lineEditInfoCategory, 4, 1);

    gridLayoutInfo->addWidget(new QLabel("PICTOGRAM"), 5, 0);
    QLineEdit* lineEditInfoPictogram = new QLineEdit;
    lineEditInfoPictogram->setObjectName("lineEditInfoPictogram");
    gridLayoutInfo->addWidget(lineEditInfoPictogram, 5, 1);

    gridLayoutInfo->addWidget(new QLabel("HINT"), 6, 0);
    QLineEdit* lineEditInfoHint = new QLineEdit;
    lineEditInfoHint->setObjectName("lineEditInfoHint");
    gridLayoutInfo->addWidget(lineEditInfoHint, 6, 1);

    gridLayoutInfo->addWidget(new QLabel("AUTHOR"), 7, 0);
    QLineEdit* lineEditInfoAuthor = new QLineEdit;
    lineEditInfoAuthor->setObjectName("lineEditInfoAuthor");
    gridLayoutInfo->addWidget(lineEditInfoAuthor, 7, 1);

    gridLayoutInfo->addWidget(new QLabel("WIKI"), 8, 0);
    QLineEdit* lineEditInfoWiki = new QLineEdit;
    lineEditInfoWiki->setObjectName("lineEditInfoWiki");
    gridLayoutInfo->addWidget(lineEditInfoWiki, 8, 1);

    QWidget* widgetSplitterInfo = new QWidget;
    widgetSplitterInfo->setLayout(gridLayoutInfo);
    gridLayoutInfo->setRowStretch(gridLayoutInfo->rowCount(), 1);

    return widgetSplitterInfo;
}

QWidget* MainWindow::CreatePropertiesTabPropertyListWidget()
{
    QHBoxLayout* hBoxLayoutPropertyListButtons = new QHBoxLayout;
    hBoxLayoutPropertyListButtons->setMargin(0);

    QLabel* labelPropertyListHeader = new QLabel;
    labelPropertyListHeader->setStyleSheet("font-weight: bold; font-size: 14px");
    labelPropertyListHeader->setText("PROPERITES");

    QToolButton* toolButtonPropertyListAdd = new QToolButton;
    toolButtonPropertyListAdd->setFixedSize(32, 32);
    toolButtonPropertyListAdd->setIconSize(QSize(32, 32));
    toolButtonPropertyListAdd->setIcon(QIcon(":/images/plus.png"));
    toolButtonPropertyListAdd->setObjectName("AAAAAAAAAA");
    toolButtonPropertyListAdd->setProperty("p1", QVariant("ssssss"));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListAdd);
    connect(toolButtonPropertyListAdd, &QToolButton::clicked, this, &MainWindow::on_toolButtonAdd_clicked);

    QToolButton* toolButtonPropertyListRemove = new QToolButton;
    toolButtonPropertyListRemove->setFixedSize(32, 32);
    toolButtonPropertyListRemove->setIconSize(QSize(32, 32));
    toolButtonPropertyListRemove->setIcon(QIcon(":/images/minus.png"));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListRemove);

    QToolButton* toolButtonPropertyListUp = new QToolButton;
    toolButtonPropertyListUp->setFixedSize(32, 32);
    toolButtonPropertyListUp->setIconSize(QSize(32, 32));
    toolButtonPropertyListUp->setIcon(QIcon(":/images/up.png"));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListUp);

    QToolButton* toolButtonPropertyListDown = new QToolButton;
    toolButtonPropertyListDown->setFixedSize(32, 32);
    toolButtonPropertyListDown->setIconSize(QSize(32, 32));
    toolButtonPropertyListDown->setIcon(QIcon(":/images/down.png"));
    hBoxLayoutPropertyListButtons->addWidget(toolButtonPropertyListDown);

    hBoxLayoutPropertyListButtons->addStretch();

    QWidget* widgetPropertyListButtons = new QWidget;
    widgetPropertyListButtons->setLayout(hBoxLayoutPropertyListButtons);






    QVBoxLayout* vBoxLayoutPropertyList = new QVBoxLayout;

    QWidget* widgetSplitterPropertyList = new QWidget;
    vBoxLayoutPropertyList->addWidget(labelPropertyListHeader, 0, Qt::AlignCenter);
    vBoxLayoutPropertyList->addWidget(widgetPropertyListButtons);
    vBoxLayoutPropertyList->addWidget(new QListWidget, 1);
    vBoxLayoutPropertyList->addStretch();

    widgetSplitterPropertyList->setLayout(vBoxLayoutPropertyList);


    return widgetSplitterPropertyList;

}

QWidget* MainWindow::CreatePropertiesTabPropertiesWidget()
{
    QGridLayout* gridLayoutProperties = new QGridLayout;

    gridLayoutProperties->addWidget(new QLabel("NAME"), 0, 0);
    QLineEdit* lineEditPropertiesName = new QLineEdit;
    lineEditPropertiesName->setObjectName("lineEditPropertiesName");
    gridLayoutProperties->addWidget(lineEditPropertiesName, 0, 1);

    gridLayoutProperties->addWidget(new QLabel("TYPE"), 1, 0);
    QLineEdit* lineEditPropertiesType = new QLineEdit;
    lineEditPropertiesType->setObjectName("lineEditPropertiesType");
    gridLayoutProperties->addWidget(lineEditPropertiesType, 1, 1);

    gridLayoutProperties->addWidget(new QLabel("DISPLAY_NAME"), 2, 0);
    QLineEdit* lineEditPropertiesDisplayName = new QLineEdit;
    lineEditPropertiesDisplayName->setObjectName("lineEditPropertiesDisplayName");
    gridLayoutProperties->addWidget(lineEditPropertiesDisplayName, 2, 1);

    gridLayoutProperties->addWidget(new QLabel("DESCRIPTION"), 3, 0);
    QPlainTextEdit* lineEditPropertiesDescription = new QPlainTextEdit;
    lineEditPropertiesDescription->setObjectName("lineEditPropertiesDescription");
    gridLayoutProperties->addWidget(lineEditPropertiesDescription, 3, 1);

    gridLayoutProperties->addWidget(new QLabel("REQUIRED"), 4, 0);
    QCheckBox* checkBoxPropertiesRequired = new QCheckBox;
    checkBoxPropertiesRequired->setObjectName("checkBoxPropertiesRequired");
    gridLayoutProperties->addWidget(checkBoxPropertiesRequired, 4, 1);

    gridLayoutProperties->addWidget(new QLabel("DEFAULT"), 5, 0);
    QLineEdit* lineEditPropertiesDefault = new QLineEdit;
    lineEditPropertiesDefault->setObjectName("lineEditPropertiesDefault");
    gridLayoutProperties->addWidget(lineEditPropertiesDefault, 5, 1);

    gridLayoutProperties->addWidget(new QLabel("HINT"), 6, 0);
    QLineEdit* lineEditPropertiesHint = new QLineEdit;
    lineEditPropertiesHint->setObjectName("lineEditPropertiesHint");
    gridLayoutProperties->addWidget(lineEditPropertiesHint, 6, 1);

    QLabel* labelPropertiesHeader = new QLabel;
    labelPropertiesHeader->setStyleSheet("font-size: 14px");
    labelPropertiesHeader->setText("RESTRICTIONS");
    gridLayoutProperties->addWidget(labelPropertiesHeader, 7, 0, 1, 2, Qt::AlignCenter);

    gridLayoutProperties->addWidget(new QLabel("MIN"), 8, 0);
    QLineEdit* lineEditPropertiesRestrictionsMin = new QLineEdit;
    lineEditPropertiesRestrictionsMin->setObjectName("lineEditPropertiesRestrictionsMin");
    gridLayoutProperties->addWidget(lineEditPropertiesRestrictionsMin, 8, 1);

    gridLayoutProperties->addWidget(new QLabel("MAX"), 9, 0);
    QLineEdit* lineEditPropertiesRestrictionsMax = new QLineEdit;
    lineEditPropertiesRestrictionsMax->setObjectName("lineEditPropertiesRestrictionsMax");
    gridLayoutProperties->addWidget(lineEditPropertiesRestrictionsMax, 9, 1);

    gridLayoutProperties->setRowStretch(gridLayoutProperties->rowCount(), 1);

    QWidget* widgetSplitterProperties = new QWidget;
    widgetSplitterProperties->setLayout(gridLayoutProperties);

    QScrollArea* scrollAreaProperties = new QScrollArea;
    scrollAreaProperties->setWidget(widgetSplitterProperties);
    scrollAreaProperties->setWidgetResizable(true);
    scrollAreaProperties->setFrameStyle(0);

    return scrollAreaProperties;
}

void MainWindow::on_toolButtonAdd_clicked()
{
    QToolButton* tb = dynamic_cast<QToolButton*>(sender());
    if (tb)
    {
        qDebug() << tb->objectName();
        qDebug() << tb->property("p1");
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

