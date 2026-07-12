#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include <QWidget>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWindow(NetworkManager* networkManager,QWidget *parent = nullptr);
    ~SearchWindow();

private slots:
    void on_searchBookPushButton_clicked();

public slots:
    void handleResponse(const Response& response);

private:
    Ui::SearchWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // SEARCHWINDOW_H
