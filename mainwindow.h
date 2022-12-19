#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

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

private slots:
    void on_toolButtonAddProperty_clicked();

    void on_listWidgetProperties_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_toolButtonRemoveProperty_clicked();

//private:
//    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
