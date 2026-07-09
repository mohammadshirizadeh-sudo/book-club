#ifndef EDITUSERINFODIALOG_H
#define EDITUSERINFODIALOG_H

#include <QDialog>

namespace Ui {
class EditUserInfoDialog;
}

class EditUserInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditUserInfoDialog(QWidget *parent = nullptr);
    ~EditUserInfoDialog();

private:
    Ui::EditUserInfoDialog *ui;
};

#endif // EDITUSERINFODIALOG_H
