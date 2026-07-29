#ifndef SHOPPINGHISTORYWINDOW_H
#define SHOPPINGHISTORYWINDOW_H

#include <QMainWindow>
#include <QVariantList>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class ShoppingHistoryWindow;
}

class ShoppingHistoryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShoppingHistoryWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~ShoppingHistoryWindow();

signals:
    void backButtonClicked();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_backButton_clicked();
    void handleResponse(const Response& response);
    void onOrderDetailsClicked();

private:
    Ui::ShoppingHistoryWindow *ui;
    NetworkManager* m_networkManager;

    void loadHistory();
    void displayHistory(const QVariantList& purchases);
    void clearLayout();
};

#endif // SHOPPINGHISTORYWINDOW_H