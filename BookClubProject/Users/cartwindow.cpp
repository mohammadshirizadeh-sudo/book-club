#include "cartwindow.h"
#include "ui_CartWindow.h"
#include "../appWindow/SessionManager.h"
#include "../Users/BookDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>

CartWindow::CartWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CartWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    m_discountLabel = new QLabel(ui->summaryFrame);
    m_discountLabel->setGeometry(0, 100, 291, 81);
    m_discountLabel->setAlignment(Qt::AlignCenter);
    m_discountLabel->setStyleSheet("font: 700 9pt \"Script MT Bold\"; font-size: 26px; color: #d32f2f; background: transparent;");
    m_discountLabel->setText("Discount: 0 T");

    ui->totalValue->setGeometry(0, 200, 291, 81);
    ui->totalValue->setStyleSheet("font: 700 9pt \"Script MT Bold\"; font-size: 36px; color: #2e7d32; background: transparent;");

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

void CartWindow::loadCart()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetCart, params);
    m_networkManager->sendRequest(request);
}

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
    // 🟢 پاسخ حذف از سبد خرید
    else if (response.getCommandType() == CommandType::RemoveFromCart) {
        if (response.isSuccess()) {
            loadCart(); // رفرش خودکار سبد خرید برای نمایش لیست جدید
        } else {
            QMessageBox::warning(this, "Error", response.getMessage());
        }
    }
    else if (response.getCommandType() == CommandType::GetBookById) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            QVariantMap bookData = resData.contains("book") ? resData["book"].toMap() : resData;
            bookData["userId"] = SessionManager::instance()->getUserId();

            QTimer::singleShot(0, this, [this, bookData]() {
                BookDetailDialog dialog(m_networkManager, bookData, this);
                dialog.exec();
                loadCart();
            });
        } else {
            QMessageBox::warning(this, "Error", "Failed to fetch book details: " + response.getMessage());
        }
    } else if (response.getCommandType() == CommandType::Checkout) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            int purchaseId = data["purchaseId"].toInt();
            double finalPrice = data["finalPrice"].toDouble();
            int totalItems = data["totalItems"].toInt();

            QString msg = QString("خرید شما با موفقیت ثبت شد! 🎉\n\n"
                                  "کد پیگیری: %1\n"
                                  "تعداد کل کتاب‌ها: %2\n"
                                  "مبلغ پرداختی: %3 تومان")
                              .arg(purchaseId)
                              .arg(totalItems)
                              .arg(finalPrice);

            QMessageBox::information(this, "تسویه‌حساب موفق", msg);

            loadCart();
        } else {
            QMessageBox::critical(this, "خطا در خرید", response.getMessage());
        }
    }
}

void CartWindow::displayCart(const QVariantMap& cartData)
{
    clearLayout();

    int totalItems = cartData["totalItems"].toInt();
    ui->cartItemsGroup->setTitle(QString("Cart Items (%1)").arg(totalItems));

    double totalDiscount = cartData["totalDiscount"].toDouble();
    m_discountLabel->setText(QString("Discount:\n-%1 T").arg(totalDiscount));

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
        infoLabel->setText(QString("📖 <b>Book ID: %1</b> &nbsp;&nbsp;|&nbsp;&nbsp; 💵 Price: %2")
                               .arg(bookId).arg(priceText));
        infoLabel->setStyleSheet("font-size: 16px; color: black; border: none; background: transparent;");
        infoLabel->setCursor(Qt::PointingHandCursor);
        infoLabel->setProperty("bookId", bookId);
        infoLabel->installEventFilter(this);

        // 🟢 دکمه حذف آیتم از سبد خرید
        QPushButton* removeButton = new QPushButton("🗑️ Remove");
        removeButton->setFixedSize(100, 35);
        removeButton->setStyleSheet(
            "QPushButton { background-color: #ffcdd2; border: 1px solid #d32f2f; border-radius: 5px; font-weight: bold; font-size: 13px; color: #c62828; }"
            "QPushButton:hover { background-color: #ef9a9a; }"
            );
        removeButton->setProperty("bookId", bookId);
        connect(removeButton, &QPushButton::clicked, this, &CartWindow::onRemoveButtonClicked);

        rowLayout->addWidget(infoLabel);
        rowLayout->addStretch();
        rowLayout->addWidget(removeButton);

        ui->cartItemsLayout->addWidget(rowWidget);
    }
    ui->cartItemsLayout->addStretch();
}

// 🟢 اسلات حذف آیتم از سبد خرید
void CartWindow::onRemoveButtonClicked()
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

bool CartWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* label = qobject_cast<QLabel*>(watched);
        if (label && label->property("bookId").isValid()) {
            int bookId = label->property("bookId").toInt();

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

void CartWindow::on_backButton_clicked()
{
    emit backButtonClicked();
}

void CartWindow::on_checkoutButton_clicked()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) {
        QMessageBox::warning(this, "Error", "User is not logged in.");
        return;
    }

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::Checkout, params);
    m_networkManager->sendRequest(request);
}