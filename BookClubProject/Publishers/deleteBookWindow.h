#ifndef DELETEBOOKWINDOW_H
#define DELETEBOOKWINDOW_H

#include <QWidget>
#include "../Network-Manger/NetworkManager.h"
#include <QListWidgetItem>
#include <QMainWindow>

namespace Ui {
class deleteBookWindow;
}

class deleteBookWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit deleteBookWindow(NetworkManager* networkManager,QMainWindow *parent = nullptr);
    ~deleteBookWindow();


protected:
    void showEvent(QShowEvent *event) override;

private slots:

    void on_booksListWidget_itemDoubleClicked(QListWidgetItem *item);

    void handleResponse(const Response& response);


private:
    void requestPublisherBooks();
    Ui::deleteBookWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // DELETEBOOKWINDOW_H
