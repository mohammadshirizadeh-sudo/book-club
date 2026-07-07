#ifndef BOOKSTATICS_H
#define BOOKSTATICS_H

#include <QWidget>

namespace Ui {
class BookStatics;
}

class BookStatics : public QWidget
{
    Q_OBJECT

public:
    explicit BookStatics(QWidget *parent = nullptr);
    ~BookStatics();

private:
    Ui::BookStatics *ui;
};

#endif // BOOKSTATICS_H
