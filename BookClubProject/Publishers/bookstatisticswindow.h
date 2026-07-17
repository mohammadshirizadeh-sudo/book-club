#ifndef BOOKSTATISTICSWINDOW_H
#define BOOKSTATISTICSWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class BookStatisticsWindow; }
QT_END_NAMESPACE

class BookStatisticsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BookStatisticsWindow(QWidget *parent = nullptr);
    ~BookStatisticsWindow();

private slots:
    void on_backButton_clicked();

private:
    Ui::BookStatisticsWindow *ui;
    void setupUI();
};

#endif // BOOKSTATISTICSWINDOW_H
