#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QGridLayout>

#include "yaml_parser_types.h"

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
    void closeEvent(QCloseEvent* event) override;

private:
    TabControls& GetTabControls(QString type);
    bool IsTabControlsExists(QString type);
    QMap<QString, QObject*>& GetControls(QString type, ControlsGroup group);
    bool RenameTabControls(QString oldType, QString newType);

private slots:
    void on_NewFile_action();
    void on_OpenFile_action();
    void on_SaveFile_action();
    void on_SaveAsFile_action();
    void on_Quit_action();
    void on_AddType_action();
    void on_RemoveType_action();

private:
    void CreateUi();
    void CreateMenu();
    QWidget* CreateMainTabWidget();
    QWidget* CreateTypeTabWidget(QString type);
    QWidget* CreateMainTabInfoWidget();
    QWidget* CreateTypeTabInfoWidget(QString type);
    QWidget* CreatePropertyListWidget(QString type);
    QWidget* CreatePropertiesWidget(QString type);
    QWidget* CreateListControlWidget(int buttonSize, QString type, ControlsGroup group, QString name);

    void AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index);
    void AddListProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddLineEditRequiredProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddGroupWidget(QWidget* groupWidget, QString name, QString type, ControlsGroup group);
    void AddComboBoxPropertyType(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddComboBoxTypeType(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);

    bool ReadCurrentFileInfo();
    bool ReadCurrentMainInfo(yaml::info_info& mi);
    bool ReadCurrentTypeInfo(QString type, yaml::type_info& ti);
    bool ReadCurrentParameter(QString type, yaml::parameter_info& pi);
    
    bool FillPropertyTypeNames();
    //bool FillTypeTypeNames();

private slots:
    void on_toolButton_clicked();
    void on_toolButtonAddProperty_clicked();
    void on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_toolButtonRemoveProperty_clicked();
    void on_editingFinished();
    void on_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
