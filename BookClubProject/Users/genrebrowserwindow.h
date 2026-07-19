#ifndef GENREBROWSERWINDOW_H
#define GENREBROWSERWINDOW_H

#include <QMainWindow>
#include <QVariantList>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class GenreBrowserWindow;
}

class GenreBrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GenreBrowserWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~GenreBrowserWindow();

signals:
    void backButtonClicked();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_backButton_clicked();
    void on_searchInput_textChanged(const QString &text);
    void handleResponse(const Response& response);

private:
    enum ViewMode { GenresMode, BooksMode };

    void requestAllGenres();
    void requestBooksByGenre(const QString& genreName);
    void displayGenres(const QVariantList& genresList);
    void displayBooks(const QVariantList& booksList);
    void clearGrid();

    Ui::GenreBrowserWindow *ui;
    NetworkManager* m_networkManager;

    ViewMode m_currentMode;
    QString m_selectedGenre;
    QVariantList m_allGenres;
    QVariantList m_currentBooks;
};

#endif