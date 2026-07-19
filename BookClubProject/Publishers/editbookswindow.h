#ifndef EDITBOOKSWINDOW_H
#define EDITBOOKSWINDOW_H

#include <QWidget>

namespace Ui {
class EditBooksWindow;
}

class EditBooksWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EditBooksWindow(QWidget *parent = nullptr);
    ~EditBooksWindow();

private slots:
    void on_genreComboBox_activated(int index);

private:
    Ui::EditBooksWindow *ui;
};

#endif // EDITBOOKSWINDOW_H
