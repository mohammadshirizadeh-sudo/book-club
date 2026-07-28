#ifndef PUBLISHEDBOOKSWINDOW_H
#define PUBLISHEDBOOKSWINDOW_H

#include <QWidget>

namespace Ui {
class PublishedBooksWindow;
}

class PublishedBooksWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PublishedBooksWindow(QWidget *parent = nullptr);
    ~PublishedBooksWindow();

private:
    Ui::PublishedBooksWindow *ui;
};

#endif // PUBLISHEDBOOKSWINDOW_H
