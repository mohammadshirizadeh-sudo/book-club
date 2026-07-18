// genrewindow.h
#ifndef GENREWINDOW_H
#define GENREWINDOW_H

#include <QWidget>
#include <QCheckBox>
#include <QVector>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Shared/Genre.h"
#include "../appWindow/SessionManager.h"

namespace Ui {
class GenreWindow;
}

class GenreWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GenreWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~GenreWindow();

signals:
    void openUserWindow();

private slots:
    void updateGenreCheckBoxes();
    void on_userEnterPushButton_clicked();
    void handleGenreResponse(const Response& response);

private:
    Ui::GenreWindow *ui;
    NetworkManager* m_networkManager;
    QVector<QCheckBox*> genreCheckBoxes;

    QVector<Genre> getSelectedGenres() const;
};

#endif // GENREWINDOW_H