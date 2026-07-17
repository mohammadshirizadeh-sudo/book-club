#ifndef SHOPPINGHISTORYWINDOW_H
#define SHOPPINGHISTORYWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ShoppingHistoryWindow; }
QT_END_NAMESPACE

class ShoppingHistoryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShoppingHistoryWindow(QWidget *parent = nullptr);
    ~ShoppingHistoryWindow();

private slots:
    void on_backButton_clicked();

private:
    Ui::ShoppingHistoryWindow *ui;
    void setupUI();
};

#endif // SHOPPINGHISTORYWINDOW_H
