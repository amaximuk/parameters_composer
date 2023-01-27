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
    
    bool HaveCurrentParameter(QString type);
    bool FillPropertyTypeNames();
    bool RenamePropertyTypeNames(QString oldName, QString newName);

    TabControls& GetTabControls(QString type);
    bool IsTabControlsExists(QString type);
    QMap<QString, QObject*>& GetControls(QString type, ControlsGroup group);
    bool RenameTabControlsType(QString oldType, QString newType);

    // !!!!!!!!!!!!!!!!!!!!!
    // INCLUDES вынести в общую часть, т.к. они могут понадобиться для DEFAULT или RESTRICTION любого параметра
    //bool FillTypeTypeNames();

    void Update();
    void UpdateMain();
    void UpdateType(QString type);

private slots:
    void on_ListControlClicked();
    void on_CurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_EditingFinished();
    void on_CurrentIndexChanged(int index);
};
#endif // MAINWINDOW_H
