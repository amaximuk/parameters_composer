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

    struct ll
    {
        QString Name;
        QList<ll*> List;
        QList<ll*> FlatList;
        ll* Parent;

        ll(QString name) :
            Name(name),
            Parent(nullptr)
        {}

        ll(QString name, ll* parent) :
            Name(name),
            Parent(parent)
        {}

        bool operator==(const ll other) const
        {
            return this->Name == other.Name;
        }
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
    void CreateMenu();

private:
    void CreateUi();
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

    bool ReadCurrentParameters(QString type, yaml::parameter_info& pi);
    bool SaveCurrentParameters(QString type);
    bool ReadCurrentMainInfo(QString type, yaml::info_info& mi);
    bool ReadCurrentTypeInfo(QString type, yaml::type_info& ti);
    bool SaveCurrentInfo(QString type);
    bool SaveCurrent();
    bool Validate();

    bool Contains(QList<ll*>& list, QString value);
    void FlatList(ll* type);
    void AddTypes(ll* type, int level);
    bool RearrangeTypes();

    bool WriteCurrent(YAML::Emitter& emitter);
    bool WriteInfo(YAML::Emitter& emitter, yaml::info_info ii);
    bool WriteType(YAML::Emitter& emitter, yaml::type_info ti);
    bool WriteParameter(YAML::Emitter& emitter, yaml::parameter_info pi);

private slots:
    void on_toolButton_clicked();
    void on_toolButtonAddProperty_clicked();
    void on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_toolButtonRemoveProperty_clicked();
    void on_editingFinished();
};
#endif // MAINWINDOW_H
