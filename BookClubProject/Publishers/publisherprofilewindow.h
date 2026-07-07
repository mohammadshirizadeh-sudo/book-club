#ifndef PUBLISHERPROFILEWINDOW_H
#define PUBLISHERPROFILEWINDOW_H

#include <QWidget>

namespace Ui {
class PublisherProfileWindow;
}

class PublisherProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PublisherProfileWindow(QWidget *parent = nullptr);
    ~PublisherProfileWindow();

private:
    Ui::PublisherProfileWindow *ui;
};

#endif // PUBLISHERPROFILEWINDOW_H
