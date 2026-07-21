#ifndef APPLYDISCOUNTWINDOW_H
#define APPLYDISCOUNTWINDOW_H

#include <QWidget>
#include <QDateTime>
#include <QVariantMap>
#include <QVariantList>

namespace Ui {
class ApplyDiscountWindow;
}

// ساختار داده‌ای نمونه برای نگهداری اطلاعات کتاب
struct BookItem {
    int id = 0;
    QString title;
    double originalPrice = 0.0;
    double currentDiscount = 0.0;
    bool isPercentage = true;
    bool hasDiscount = false;
    bool isTimed = false;
    QDateTime startDate;
    QDateTime endDate;
    QString coverPath;
};

class ApplyDiscountWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ApplyDiscountWindow(QWidget *parent = nullptr);
    ~ApplyDiscountWindow();

    // متدی برای بارگذاری کتاب‌های نویسنده/کاربر در ComboBox
    void loadBooksData();

private slots:
    void on_quitPushButton_clicked();
    void on_clearFormButton_clicked();
    void on_applyDiscountButton_clicked();
    void on_removeDiscountButton_clicked();

    void on_bookComboBox_currentIndexChanged(int index);
    void on_percentageRadio_toggled(bool checked);
    void on_timedDiscountCheck_toggled(bool checked);

    // اسلات محاسبه قیمت جدید هنگام تغییر هر یک از مقادیر
    void updatePricePreview();

private:
    Ui::ApplyDiscountWindow *ui;
    QList<BookItem> m_books; // لیست کتاب‌های بارگذاری شده

    void setupUiDefaults();
    void setupConnections();
    void refreshActiveDiscountsTable();
    void clearForm();
};

#endif // APPLYDISCOUNTWINDOW_H