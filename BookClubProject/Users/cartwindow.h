#ifndef CARTWINDOW_H
#define CARTWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class CartWindow; }
QT_END_NAMESPACE

class CartWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CartWindow(QWidget *parent = nullptr);
    ~CartWindow();

private slots:
    void on_backButton_clicked();
    void on_checkoutButton_clicked();

private:
    Ui::CartWindow *ui;
    void setupUI();
};

#endif // CARTWINDOW_H
