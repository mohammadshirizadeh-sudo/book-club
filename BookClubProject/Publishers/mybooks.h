#ifndef MYBOOKS_H
#define MYBOOKS_H

#include <QWidget>

namespace Ui {
class MyBooks;
}

class MyBooks : public QWidget
{
    Q_OBJECT

public:
    explicit MyBooks(QWidget *parent = nullptr);
    ~MyBooks();

private slots:
    void on_addBookPushButton_clicked();

    void on_editBookPushButton_clicked();

    void on_staticsPushButton_clicked();

    void on_applyDiscountPushButton_clicked();

    void on_deleteDeactivatePushButton_clicked();

    void on_backPushButton_clicked();

private:
    Ui::MyBooks *ui;
};

#endif // MYBOOKS_H
