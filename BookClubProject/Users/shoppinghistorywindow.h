#ifndef SHOPPINGHISTORYWINDOW_H
#define SHOPPINGHISTORYWINDOW_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class ShoppingHistoryWindow;
}

class ShoppingHistoryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ShoppingHistoryWindow(QWidget *parent = nullptr);
    ~ShoppingHistoryWindow();

    // پاک کردن جدول
    void clearHistory();

    // اضافه کردن یک خرید
    void addPurchase(const QString &bookName,
                     const QString &author,
                     const QString &amount);

private:
    Ui::ShoppingHistoryWindow *ui;

    QStandardItemModel *model;
};

#endif // SHOPPINGHISTORYWINDOW_H