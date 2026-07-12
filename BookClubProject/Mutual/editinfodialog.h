#ifndef EDITINFODIALOG_H
#define EDITINFODIALOG_H

#include <QDialog>

namespace Ui {
class EditInfoDialog;
}

class EditInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditInfoDialog(QWidget *parent = nullptr);
    ~EditInfoDialog();

private:
    Ui::EditInfoDialog *ui;
};

#endif // EDITINFODIALOG_H
