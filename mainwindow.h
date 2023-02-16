#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QGridLayout>
#include <QTabWidget>
#include <QPlainTextEdit>

#include "focus_filter.h"
#include "parameters_compiler_types.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    enum class ControlsGroup
    {
        Info,
        Parameters,
        Properties
    };

    struct TabControls
    {
        QString Name;
        QMap<QString, QObject*> Info;
        QMap<QString, QObject*> Parameters;
        QMap<QString, QObject*> Properties;
    };

private:
    QString currentFileName_;
    parameters_compiler::file_info fileInfo_;
    QList<TabControls> tabs_;
    bool is_json_;
    bool modified_;
    QTabWidget* tabWidget_;
    QPlainTextEdit* plainTextEditHint_;
    FocusFilter* focusFilter_;
    QMenu* recentMenu_;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool OpenFile(QString fileName);

private:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_NewFile_action();
    void on_OpenFile_action();
    void on_SaveFile_action();
    void on_SaveAsFile_action();
    void on_Apply_action();
    void on_Quit_action();
    void on_AddType_action();
    void on_RemoveType_action();
    void on_Compile_action();
    void on_ViewYaml_action();
    void on_ViewJson_action();
    void on_ViewCode_action();
    void on_ViewHtml_action();
    void on_OpenFolder_action();
    void on_Help_action();
    void on_Recent_action();

private:
    void CreateUi();
    void CreateMenu();
    QWidget* CreateMainWidget();
    QWidget* CreateMainTabWidget();
    QWidget* CreateTypeTabWidget(QString type);
    QWidget* CreateMainTabInfoWidget();
    QWidget* CreateTypeTabInfoWidget(QString type);
    QWidget* CreatePropertyListWidget(QString type);
    QWidget* CreatePropertiesWidget(QString type);
    QWidget* CreateListControlWidget(int buttonSize, QString type, ControlsGroup group, QString name, QString toolTipBase);

    void AddLineEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group, bool bold);
    void AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index);
    void AddListProperty(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddGroupWidget(QWidget* groupWidget, QString name, QString type, ControlsGroup group);
    void AddComboBoxPropertyType(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);
    void AddComboBoxTypeType(QGridLayout* gridLayout, QString name, int index, QString type, ControlsGroup group);

    bool ReadCurrentFileInfo();
    bool ReadCurrentMainInfo(parameters_compiler::info_info& mi);
    bool ReadCurrentTypeInfo(QString type, parameters_compiler::type_info& ti);
    bool ReadCurrentParameter(QString type, parameters_compiler::parameter_info& pi);
    
    bool HaveCurrentParameter(QString type);
    bool FillPropertyTypeNames();
    bool RenamePropertyTypeNames(QString oldName, QString newName);

    TabControls& GetTabControls(QString type);
    bool IsTabControlsExists(QString type);
    QMap<QString, QObject*>& GetControls(QString type, ControlsGroup group);
    bool RenameTabControlsType(QString oldType, QString newType);

    // !!!!!!!!!!!!!!!!!!!!!
    // INCLUDES вынести в общую часть, т.к. они могут понадобиться для DEFAULT или RESTRICTION любого параметра
    // это не очень бьется со структурой файла, так что скорее всего останется как есть
    // если нужен заголовочник, можно создать пустой тип enum
    // !!!!!!!!!!!!!!!!!!!!!
    // REQUIRED bool->string?
    // !!! remove -> last error???


    void Update();
    void UpdateMain();
    void UpdateType(QString type);
    void UpdateWindowTitle();

    void SaveAs();
    bool SaveAsInternal(QString fileName, bool is_json, bool is_temp);
    bool Compile();
    bool OpenFileInternal(QString fileName, bool is_json);
    bool ApplyInternal(QString fileName);
    void UpdateRecent();
    void AddRecent(QString fileName);
    void RemoveRecent(QString fileName);

private slots:
    void on_ListControlClicked();
    void on_CurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_EditingFinished();
    void on_CurrentIndexChanged(int index);
    void on_TextChanged();
    void on_StateChanged(int state);
    void on_FocusChanged(QObject* sender, bool focus);
};
#endif // MAINWINDOW_H
