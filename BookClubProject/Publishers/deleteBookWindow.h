#ifndef DELETEBOOKWINDOW_H
#define DELETEBOOKWINDOW_H

#include <QWidget>

namespace Ui {
class deleteBookWindow;
}

class deleteBookWindow : public QWidget
{
    Q_OBJECT

public:
    explicit deleteBookWindow(QWidget *parent = nullptr);
    ~deleteBookWindow();

private:
    Ui::deleteBookWindow *ui;
};

#endif // DELETEBOOKWINDOW_H
