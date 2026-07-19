#ifndef FAVORITEBOOKSWINDOW_H
#define FAVORITEBOOKSWINDOW_H

#include <QMainWindow>
#include <QVariantList>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class FavoriteBooksWindow;
}

class FavoriteBooksWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FavoriteBooksWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~FavoriteBooksWindow();

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
    void loadFavoriteBooks();
    void displayBooks(const QVariantList& books);
    void clearGrid();

    Ui::FavoriteBooksWindow *ui;
    NetworkManager* m_networkManager;
    QVariantList m_allFavoriteBooks;
};

#endif // FAVORITEBOOKSWINDOW_H