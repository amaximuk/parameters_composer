#include <QDebug>
#include <QInputDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/parameters.png"));

    ui->toolButtonAddProperty->setFixedSize(32, 32);
    ui->toolButtonAddProperty->setIconSize(QSize(32, 32));
    ui->toolButtonAddProperty->setIcon(QIcon(":/images/plus.png"));

    ui->toolButtonRemoveProperty->setFixedSize(32, 32);
    ui->toolButtonRemoveProperty->setIconSize(QSize(32, 32));
    ui->toolButtonRemoveProperty->setIcon(QIcon(":/images/minus.png"));

    ui->toolButtonUpProperty->setFixedSize(32, 32);
    ui->toolButtonUpProperty->setIconSize(QSize(32, 32));
    ui->toolButtonUpProperty->setIcon(QIcon(":/images/up.png"));

    ui->toolButtonDownProperty->setFixedSize(32, 32);
    ui->toolButtonDownProperty->setIconSize(QSize(32, 32));
    ui->toolButtonDownProperty->setIcon(QIcon(":/images/down.png"));




    ui->toolButtonPropertyRestrictionSetAdd->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionSetAdd->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionSetAdd->setIcon(QIcon(":/images/plus.png"));

    ui->toolButtonPropertyRestrictionSetRemove->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionSetRemove->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionSetRemove->setIcon(QIcon(":/images/minus.png"));

    ui->toolButtonPropertyRestrictionSetUp->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionSetUp->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionSetUp->setIcon(QIcon(":/images/up.png"));

    ui->toolButtonPropertyRestrictionSetDown->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionSetDown->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionSetDown->setIcon(QIcon(":/images/down.png"));





    ui->toolButtonPropertyRestrictionCountSetAdd->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionCountSetAdd->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionCountSetAdd->setIcon(QIcon(":/images/plus.png"));

    ui->toolButtonPropertyRestrictionCountSetRemove->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionCountSetRemove->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionCountSetRemove->setIcon(QIcon(":/images/minus.png"));

    ui->toolButtonPropertyRestrictionCountSetUp->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionCountSetUp->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionCountSetUp->setIcon(QIcon(":/images/up.png"));

    ui->toolButtonPropertyRestrictionCountSetDown->setFixedSize(24, 24);
    ui->toolButtonPropertyRestrictionCountSetDown->setIconSize(QSize(24, 24));
    ui->toolButtonPropertyRestrictionCountSetDown->setIcon(QIcon(":/images/down.png"));





    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_toolButtonAddProperty_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Property", "Property name:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(text);
        ui->listWidgetProperties->addItem(newItem);
    }
}


void MainWindow::on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current != 0)
        qDebug() << current->text();

}


void MainWindow::on_toolButtonRemoveProperty_clicked()
{
    auto si = ui->listWidgetProperties->selectedItems();
    if (si.size() > 0)
    {
        delete si[0];
    }
}

