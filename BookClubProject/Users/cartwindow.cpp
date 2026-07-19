
#include "cartwindow.h"
#include "ui_CartWindow.h"
#include "../appWindow/SessionManager.h"
#include "../Users/BookDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>

CartWindow::CartWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CartWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    m_discountLabel = new QLabel(ui->summaryFrame);
    m_discountLabel->setGeometry(0, 100, 291, 81); // قرارگیری در بخش بالایی باکس سمت راست
    m_discountLabel->setAlignment(Qt::AlignCenter);
    m_discountLabel->setStyleSheet("font: 700 9pt \"Script MT Bold\"; font-size: 26px; color: #d32f2f; background: transparent;");
    m_discountLabel->setText("Discount: 0 T");

    // تنظیم استایل مجدد قیمت نهایی برای زیبایی بیشتر و هماهنگی با لیبل تخفیف
    ui->totalValue->setGeometry(0, 200, 291, 81);
    ui->totalValue->setStyleSheet("font: 700 9pt \"Script MT Bold\"; font-size: 36px; color: #2e7d32; background: transparent;");

    // اتصال سیگنال پاسخ شبکه
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &CartWindow::handleResponse);
}

CartWindow::~CartWindow()
{
    delete ui;
}

void CartWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    loadCart();
}

// ۱. درخواست دیتای سبد خرید از سرور
void CartWindow::loadCart()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetCart, params);
    m_networkManager->sendRequest(request);
}

// ۲. مدیریت پاسخ دریافتی سرور
void CartWindow::handleResponse(const Response& response)
{
    if (response.getCommandType() == CommandType::GetCart) {
        if (response.isSuccess()) {
            QVariantMap cartData = response.getData();
            displayCart(cartData);
        } else {
            QMessageBox::critical(this, "Error", "Failed to load cart.");
        }
    }
    // ۲. 🟢 پاسخ حذف کتاب از سبد خرید
    else if (response.getCommandType() == CommandType::RemoveFromCart) {
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Item removed from cart successfully!");
            loadCart(); // رفرش و لود مجدد سبد خرید از سرور برای به‌روزرسانی قیمت‌ها
        } else {
            QMessageBox::warning(this, "Error", response.getMessage());
        }
    }

    else if (response.getCommandType() == CommandType::GetBookById) {

        if (response.isSuccess()) {

            QVariantMap resData = response.getData();
            QVariantMap bookData =
                resData.contains("book")
                    ? resData["book"].toMap()
                    : resData;

            bookData["userId"] =
                SessionManager::instance()->getUserId();

            int bookId = bookData["bookId"].toInt();

            QTimer::singleShot(0, this,
                               [this, bookData]()
                               {
                                   BookDetailDialog dialog(
                                       m_networkManager,
                                       bookData,
                                       this
                                       );

                                   dialog.exec();

                                   loadCart();
                               });
        }
        else {
            QMessageBox::warning(
                this,
                "Error",
                "Failed to fetch book details: "
                    + response.getMessage()
                );
        }
    }
}
void CartWindow::displayCart(const QVariantMap& cartData)
{
    clearLayout();

    int totalItems = cartData["totalItems"].toInt();
    ui->cartItemsGroup->setTitle(QString("Cart Items (%1)").arg(totalItems));

    // 🟢 به‌روزرسانی مقدار تخفیف کل در باکس سمت راست
    double totalDiscount = cartData["totalDiscount"].toDouble();
    m_discountLabel->setText(QString("Discount:\n-%1 T").arg(totalDiscount));

    // به‌روزرسانی قیمت نهایی در باکس سمت راست
    double finalPrice = cartData["finalPrice"].toDouble();
    ui->totalValue->setText(QString("Total:\n%1 T").arg(finalPrice));

    if (cartData["isEmpty"].toBool() || totalItems == 0) {
        QLabel* emptyLabel = new QLabel("Your shopping cart is empty.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 20px; color: gray; font-weight: bold; margin-top: 50px;");
        ui->cartItemsLayout->addWidget(emptyLabel);
        return;
    }

    QVariantList items = cartData["items"].toList();
    for (const QVariant& var : items) {
        QVariantMap item = var.toMap();

        QWidget* rowWidget = new QWidget();
        rowWidget->setStyleSheet("QWidget { background-color: #f9f9f9; border: 2px solid black; border-radius: 8px; }");

        QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(15, 10, 15, 10);

        int bookId = item["bookId"].toInt();
        int quantity = item["quantity"].toInt();
        double totalPrice = item["totalPrice"].toDouble();
        double totalDiscountedPrice = item["totalDiscountedPrice"].toDouble();

        QString priceText;
        if (totalDiscountedPrice < totalPrice) {
            priceText = QString("<font color='gray'><del>%1</del></font> <font color='green'><b>%2 Tooman</b></font>")
            .arg(totalPrice).arg(totalDiscountedPrice);
        } else {
            priceText = QString("<b>%1 Tooman</b>").arg(totalPrice);
        }

        // لیبل متن اطلاعات کتاب
        QLabel* infoLabel = new QLabel();
        infoLabel->setText(QString("      📖 <b>Book ID: %1</b> &nbsp;&nbsp;|&nbsp;&nbsp; 📦 Qty: <b>%2</b> &nbsp;&nbsp;|&nbsp;&nbsp; 💵 Total: %3")
                               .arg(bookId).arg(quantity).arg(priceText));
        infoLabel->setStyleSheet("font-size: 16px; color: black; border: none; background: transparent;");

        // 🟢 تنظیمات کلیک روی متن برای باز شدن اطلاعات کتاب
        infoLabel->setCursor(Qt::PointingHandCursor);
        infoLabel->setProperty("bookId", bookId); // ذخیره آیدی کتاب درون این لیبل
        infoLabel->installEventFilter(this); // اتصال به فیلتر رویداد کلیک

        // 🟢 ساخت دکمه حذف از لیست (❌) برای هر ردیف
        QPushButton* deleteButton = new QPushButton("❌ Remove");
        deleteButton->setFixedSize(110, 35);
        deleteButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #ffcdd2;"
            "  border: 2px solid #d32f2f;"
            "  border-radius: 6px;"
            "  color: #c62828;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "  background-color: #ef9a9a;"
            "}"
            );
        deleteButton->setProperty("bookId", bookId); // ذخیره آیدی کتاب درون دکمه حذف
        connect(deleteButton, &QPushButton::clicked, this, &CartWindow::onDeleteButtonClicked);

        rowLayout->addWidget(infoLabel);
        rowLayout->addStretch();
        rowLayout->addWidget(deleteButton); // اضافه کردن دکمه حذف به انتهای ردیف

        ui->cartItemsLayout->addWidget(rowWidget);
    }
    ui->cartItemsLayout->addStretch();
}

bool CartWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* label = qobject_cast<QLabel*>(watched);
        if (label && label->property("bookId").isValid()) {
            int bookId = label->property("bookId").toInt();

            // ارسال درخواست به سرور جهت گرفتن اطلاعات کامل کتاب برای دیالوگ
            QVariantMap params;
            params["bookId"] = bookId;
            params["userId"] = SessionManager::instance()->getUserId();

            Request request(CommandType::GetBookById, params);
            m_networkManager->sendRequest(request);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
void CartWindow::clearLayout()
{
    QLayoutItem *item;
    while ((item = ui->cartItemsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void CartWindow::onDeleteButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int bookId = button->property("bookId").toInt();
    int userId = SessionManager::instance()->getUserId();

    if (userId <= 0 || bookId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;
    params["bookId"] = bookId;

    Request request(CommandType::RemoveFromCart, params);
    m_networkManager->sendRequest(request);
}

// مدیریت دکمه بازگشت
void CartWindow::on_backButton_clicked()
{
    emit backButtonClicked();
}

// مدیریت دکمه ثبت نهایی/پرداخت
void CartWindow::on_checkoutButton_clicked()
{
    QMessageBox::information(this, "Checkout", "Proceeding to order finalization...");
}