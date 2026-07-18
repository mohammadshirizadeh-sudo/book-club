#ifndef REVIEWBOOKDIALOG_H
#define REVIEWBOOKDIALOG_H

#include <QDialog>

namespace Ui {
class ReviewBookDialog;
}

class ReviewBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReviewBookDialog(QWidget *parent = nullptr);
    ~ReviewBookDialog();

private:
    Ui::ReviewBookDialog *ui;
};

#endif // REVIEWBOOKDIALOG_H
