#ifndef USERPROFILEWINDOW_H
#define USERPROFILEWINDOW_H

#include <QWidget>

namespace Ui {
class UserProfileWindow;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

private:
    Ui::UserProfileWindow *ui;
};

#endif // USERPROFILEWINDOW_H
