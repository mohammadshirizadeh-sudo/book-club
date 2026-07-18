#ifndef GENREBROWSERWINDOW_H
#define GENREBROWSERWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class GenreBrowserWindow; }
QT_END_NAMESPACE

class GenreBrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GenreBrowserWindow(QWidget *parent = nullptr);
    ~GenreBrowserWindow();

private slots:
    void on_backButton_clicked();

private:
    Ui::GenreBrowserWindow *ui;
    void setupUI();
};

#endif // GENREBROWSERWINDOW_H
