#ifndef APPLYDISCOUNTWINDOW_H
#define APPLYDISCOUNTWINDOW_H

#include <QWidget>
#include <QDateTime>

namespace Ui {
class ApplyDiscountWindow;
}

class ApplyDiscountWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyDiscountWindow(QWidget *parent = nullptr);
    ~ApplyDiscountWindow();

signals:
    void showHomePageRequested();
    void showProfileRequested();
    void showMyBooksRequested();
    void signOutRequested();
    void showAddNewBookRequested();
    void showEditBooksRequested();
    void showBookStaticsRequested();
    void showDeactivateBookRequested();
    void showNotificationsRequested();

private slots:
    void on_bookComboBox_currentIndexChanged(int index);
    void on_percentageRadio_toggled(bool checked);
    void on_fixedAmountRadio_toggled(bool checked);
    void on_percentageSpinBox_valueChanged(int value);
    void on_fixedAmountSpinBox_valueChanged(double value);
    void on_timedDiscountCheck_toggled(bool checked);
    void on_applyDiscountButton_clicked();
    void on_removeDiscountButton_clicked();
    void on_clearFormButton_clicked();

private:
    Ui::ApplyDiscountWindow *ui;

    // Current selected book data
    int m_currentBookId;
    double m_currentBookPrice;
    QString m_currentBookTitle;

    // Data methods
    void loadBooksList();
    void updateBookInfo(int bookIndex);
    void updateNewPricePreview();
    void applyDiscountToBook();
    void removeDiscountFromBook();
    void clearForm();
    void loadActiveDiscounts();
    bool validateInputs();

    // Database methods (to be connected)
    QStringList getPublisherBooks() const;
    double getBookPrice(int bookId) const;
    QString getBookTitle(int bookId) const;
    bool saveDiscountToDatabase(int bookId, const QString &discountType,
                                double discountValue,
                                const QDateTime &startDate,
                                const QDateTime &endDate);
    bool removeDiscountFromDatabase(int bookId);
};

#endif // APPLYDISCOUNTWINDOW_H
