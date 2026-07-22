#ifndef APPLYDISCOUNTWINDOW_H
#define APPLYDISCOUNTWINDOW_H

#include <QWidget>
#include <QVariantMap>
#include <QList>
#include <QDateTime>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Response.h"

namespace Ui {
class ApplyDiscountWindow;
}

class ApplyDiscountWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyDiscountWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~ApplyDiscountWindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void handleResponse(const Response& response);
    void on_applyDiscountButton_clicked();
    void on_removeDiscountButton_clicked();
    void on_clearFormButton_clicked();
    void on_quitPushButton_clicked();

    void on_bookComboBox_currentIndexChanged(int index);
    void on_percentageRadio_toggled(bool checked);
    void on_fixedAmountRadio_toggled(bool checked);
    void on_timedDiscountCheck_toggled(bool checked);
    void on_percentageSpinBox_valueChanged(int value);
    void on_fixedAmountSpinBox_valueChanged(double value);

private:
    Ui::ApplyDiscountWindow *ui;
    NetworkManager *m_networkManager;

    QList<QVariantMap> m_publisherBooks;
    int m_selectedBookId = -1;


    void loadPublisherBooks();
    void updatePricePreview();
    void updateBookDetailsUI(const QVariantMap& bookData);
    void updateActiveDiscountsTable();
    void clearForm();
};

#endif // APPLYDISCOUNTWINDOW_H