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
    enum class ControlsGroup
    {
        Info,
        PropertyList,
        Properties
    };

    struct TabControls
    {
        QString Name;
        QMap<QString, QObject*> Info;
        QMap<QString, QObject*> PropertyList;
        QMap<QString, QObject*> Properties;
    };

private:
    QString currentFileName_;
    yaml::file_info fileInfo_;
    QList<TabControls> tabs_;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    TabControls& GetTabControls(QString type);
    bool IsTabControlsExists(QString type);
    QMap<QString, QObject*>& GetControls(QString type, ControlsGroup group);

private slots:
    void on_NewFile_action();
    void on_OpenFile_action();
    void on_SaveFile_action();
    void on_SaveAsFile_action();
    void on_Quit_action();

private:
    void CreateMenu();

private:
    void CreateUi();
    QWidget* CreateMainTabWidget();
    QWidget* CreateTypeTabWidget(QString type);

    QWidget* CreateMainTabInfoWidget();
    QWidget* CreateTypeTabInfoWidget(QString type);

    QWidget* CreatePropertyListWidget(QString type);
    QWidget* CreatePropertiesWidget(QString type);

    QWidget* CreateListControlWidget(int buttonSize, QString tabId, QString listControlId, QString type);




    void AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls);
    void AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls);
    void AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QMap<QString, QObject*>& mapControls);
    void AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index);
    void AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId, QMap<QString, QObject*>& mapControls, QString type);

    void AddLineEditRequiredProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);

    bool ReadCurrentParameters(QString type, yaml::parameter_info& pi);

private slots:
    void on_toolButton_clicked();
    void on_toolButtonAddProperty_clicked();
    void on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_toolButtonRemoveProperty_clicked();
    void on_editingFinished();
};
#endif // MAINWINDOW_H
