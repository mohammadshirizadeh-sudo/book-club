#ifndef GENREWINDOW_H
#define GENREWINDOW_H

#include <QWidget>
#include <QCheckBox>

namespace Ui {
class GenreWindow;
}

class GenreWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GenreWindow(QWidget *parent = nullptr);
    ~GenreWindow();

signals:
    void openUserWindow();

private slots:
    void updateGenreCheckBoxes();
    void on_userEnterPushButton_clicked();

private:
    Ui::GenreWindow *ui;

    QList<QCheckBox*> genreCheckBoxes;
};

#endif // GENREWINDOW_H
