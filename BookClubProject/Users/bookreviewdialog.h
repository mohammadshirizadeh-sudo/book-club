#ifndef BOOKREVIEWDIALOG_H
#define BOOKREVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class BookReviewDialog;
}

class BookReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookReviewDialog(QWidget *parent = nullptr);
    ~BookReviewDialog();

private:
    Ui::BookReviewDialog *ui;
};

#endif // BOOKREVIEWDIALOG_H
