#ifndef MYBOOKS_H
#define MYBOOKS_H

#include <QWidget>

namespace Ui {
class MyBooks;
}

class MyBooks : public QWidget
{
    Q_OBJECT

public:
    explicit MyBooks(QWidget *parent = nullptr);
    ~MyBooks();

private:
    Ui::MyBooks *ui;
};

#endif // MYBOOKS_H
