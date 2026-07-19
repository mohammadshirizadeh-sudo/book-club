#ifndef CARTWINDOW_H
#define CARTWINDOW_H

#include <QMainWindow>
#include <QVariantMap>
#include "../Network-Manger/NetworkManager.h"
#include <QLabel>

namespace Ui {
class CartWindow;
}

class CartWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CartWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~CartWindow();

signals:
    void backButtonClicked(); // سیگنال بازگشت به منوی اصلی

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_backButton_clicked();
    void on_checkoutButton_clicked();
    void handleResponse(const Response& response);
    void onDeleteButtonClicked();

private:
    void loadCart(); // ارسال درخواست به سرور
    void displayCart(const QVariantMap& cartData); // رندر کردن آیتم‌ها و قیمت‌ها
    void clearLayout(); // پاکسازی لیست برای لود مجدد

    Ui::CartWindow *ui;
    NetworkManager* m_networkManager;
    QLabel* m_discountLabel;
};

#endif // CARTWINDOW_H