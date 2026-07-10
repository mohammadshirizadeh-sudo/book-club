#ifndef CHANGEUSERPASSDIALOG_H
#define CHANGEUSERPASSDIALOG_H

#include <QDialog>

namespace Ui {
class ChangeUserPassDialog;
}

class ChangeUserPassDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserPassDialog(QWidget *parent = nullptr);
    ~ChangeUserPassDialog();

private:
    Ui::ChangeUserPassDialog *ui;
};

#endif // CHANGEUSERPASSDIALOG_H
