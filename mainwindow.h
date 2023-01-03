#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QGridLayout>

//QT_BEGIN_NAMESPACE
//namespace Ui { class MainWindow; }
//QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void CreateUi();
    QWidget* CreatePropertiesTabWidget();
    QWidget* CreatePropertiesTabInfoWidget();
    QWidget* CreatePropertiesTabPropertyListWidget();
    QWidget* CreatePropertiesTabPropertiesWidget();
    QWidget* CreateToolBoxListControlWidget(int buttonSize, QString tabId, QString listControlId);
    QWidget* CreateYmlTypeTabWidget();
    QWidget* CreateYmlTypeTabInfoWidget();
    QWidget* CreateCppTypeTabWidget();

    void AddLineEditProperty(QGridLayout* gridLayout, QString name, int index);
    void AddPlainTextEditProperty(QGridLayout* gridLayout, QString name, int index);
    void AddCheckBoxProperty(QGridLayout* gridLayout, QString name, int index);
    void AddPropertySubheader(QGridLayout* gridLayout, QString text, QString style, int index);
    void AddListProperty(QGridLayout* gridLayout, QString name, int index, QString tabId, QString listControlId);

private slots:
    void on_toolButton_clicked();
    void on_toolButtonAddProperty_clicked();
    void on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_toolButtonRemoveProperty_clicked();

//private:
//    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
