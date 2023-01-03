#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QGridLayout>

#include "yaml_parser.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    struct TabControls
    {
        QMap<QString, QObject*> Info;
        QMap<QString, QObject*> PropertyList;
        QMap<QString, QObject*> Properties;
    };

private:
    QString currentFileName_;
    yaml::yaml_parser* parser_;
    QMap<QString, TabControls> tabs_;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_NewFile_action();
    void on_OpenFile_action();
    void on_Quit_action();

private:
    void CreateMenu();

private:
    void CreateUi();
    QWidget* CreateMainTabWidget();
    QWidget* CreateTypeTabWidget(QString typeName);

    QWidget* CreateMainTabInfoWidget();
    QWidget* CreateTypeTabInfoWidget(QString typeName);

    QWidget* CreatePropertyListWidget(QString typeName);
    QWidget* CreatePropertiesWidget(QString typeName);

    QWidget* CreateListControlWidget(int buttonSize, QString tabId, QString listControlId, QString typeName);




    void AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls);
    void AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls);
    void AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls);
    void AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index, QMap<QString, QObject*>& mapControls);
    void AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId, QMap<QString, QObject*>& mapControls, QString typeName);

private slots:
    void on_toolButton_clicked();
    void on_toolButtonAddProperty_clicked();
    void on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_toolButtonRemoveProperty_clicked();
};
#endif // MAINWINDOW_H
