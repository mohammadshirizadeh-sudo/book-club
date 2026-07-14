#ifndef ALLGENRESWINDOW_H
#define ALLGENRESWINDOW_H

#include <QWidget>

namespace Ui {
class AllGenresWindow;
}

class AllGenresWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AllGenresWindow(QWidget *parent = nullptr);
    ~AllGenresWindow();

private:
    Ui::AllGenresWindow *ui;
};

#endif // ALLGENRESWINDOW_H
