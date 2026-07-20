#ifndef MYLIBRARYWINDOW_H
#define MYLIBRARYWINDOW_H

#include <QMainWindow>
#include <QVariantList>
#include "../Network-Manger//NetworkManager.h"

namespace Ui {
class MyLibraryWindow;
}

class MyLibraryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MyLibraryWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~MyLibraryWindow();

signals:
    void backButtonClicked();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_backButton_clicked();
    void onSearchTextChanged(const QString &text);
    void handleResponse(const Response& response);

private:
    Ui::MyLibraryWindow *ui;
    NetworkManager* m_networkManager;
    QVariantList m_allBooks; // نگهداری لیست کل کتاب‌ها برای فیلتر و جستجو

    void loadLibrary();
    void displayBooks(const QVariantList& books);
    void filterBooks(const QString& query);
    void clearLayout();
};

#endif // MYLIBRARYWINDOW_H