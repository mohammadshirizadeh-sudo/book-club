#ifndef BOOKDETAILDIALOG_H
#define BOOKDETAILDIALOG_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class BookDetailDialog;
}

class BookDetailDialog : public QDialog
{
    Q_OBJECT

public:
    // سازنده را طوری تغییر می‌دهیم که اطلاعات کتاب (bookData) را ورودی بگیرد
    explicit BookDetailDialog(const QVariantMap& bookData, QWidget *parent = nullptr);
    ~BookDetailDialog();

private:
    Ui::BookDetailDialog *ui;
    void displayBookInfo(const QVariantMap& bookData);
};

#endif // BOOKDETAILDIALOG_H