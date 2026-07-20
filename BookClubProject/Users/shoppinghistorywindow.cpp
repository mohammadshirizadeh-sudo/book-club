#include "shoppinghistorywindow.h"
#include "ui_ShoppingHistoryWindow.h"
#include "../appWindow/SessionManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QDateTime>

ShoppingHistoryWindow::ShoppingHistoryWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShoppingHistoryWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    // اتصال سیگنال دریافت پاسخ از شبکه
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &ShoppingHistoryWindow::handleResponse);
}

ShoppingHistoryWindow::~ShoppingHistoryWindow()
{
    delete ui;
}

// 🟢 هنگام نمایش صفحه، تاریخچه خریدها لود می‌شود
void ShoppingHistoryWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    loadHistory();
}

// 🟢 ارسال درخواست دریافت تاریخچه به سرور
void ShoppingHistoryWindow::loadHistory()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetPurchaseHistory, params);
    m_networkManager->sendRequest(request);
}

// 🟢 دریافت و پردازش پاسخ سرور
void ShoppingHistoryWindow::handleResponse(const Response& response)
{
    // ۱. پاسخ لیست تاریخچه خریدها
    if (response.getCommandType() == CommandType::GetPurchaseHistory) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            QVariantList purchases = data["purchases"].toList();
            displayHistory(purchases);
        } else {
            QMessageBox::warning(this, "Error", "Failed to load purchase history: " + response.getMessage());
        }
    }
    // ۲. پاسخ جزئیات یک سفارش خاص (در صورت ارسال درخواست جزئیات)
    else if (response.getCommandType() == CommandType::GetPurchaseById) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            int purchaseId = data["purchaseId"].toInt();
            double totalPrice = data["totalPrice"].toDouble();
            double discountAmount = data["discountAmount"].toDouble();
            double finalPrice = data["finalPrice"].toDouble();
            QVariantList items = data["items"].toList();

            QString detailsText = QString("📦 <b>Order ID: %1</b><br>"
                                          "💵 Original Price: %2 T<br>"
                                          "🎁 Discount: %3 T<br>"
                                          "✅ Final Paid: <b>%4 T</b><br><br>"
                                          "<b>Purchased Books:</b><br>")
                                      .arg(purchaseId)
                                      .arg(totalPrice)
                                      .arg(discountAmount)
                                      .arg(finalPrice);

            if (items.isEmpty()) {
                detailsText += "<i>No item details found.</i>";
            } else {
                for (const QVariant& var : items) {
                    QVariantMap item = var.toMap();
                    detailsText += QString("• 📖 <b>%1</b> (Qty: %2) - %3 T<br>")
                                       .arg(item["title"].toString().isEmpty() ? QString("Book ID: %1").arg(item["bookId"].toInt()) : item["title"].toString())
                                       .arg(item["quantity"].toInt())
                                       .arg(item["price"].toDouble());
                }
            }

            QMessageBox::information(this, "Order Details", detailsText);
        } else {
            QMessageBox::warning(this, "Error", response.getMessage());
        }
    }
}

// 🟢 نمایش سفارش‌ها در UI
void ShoppingHistoryWindow::displayHistory(const QVariantList& purchases)
{
    clearLayout();

    double totalSpentSum = 0.0;

    if (purchases.isEmpty()) {
        QLabel* emptyLabel = new QLabel("You have no purchase history yet.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 20px; color: gray; font-weight: bold; margin-top: 50px;");
        ui->ordersLayout->addWidget(emptyLabel);
        ui->totalSpentValueLabel->setText("0 T");
        return;
    }

    for (const QVariant& var : purchases) {
        QVariantMap purchase = var.toMap();

        int purchaseId = purchase["purchaseId"].toInt();
        double finalPrice = purchase["finalPrice"].toDouble();
        int totalItems = purchase["totalItems"].toInt();
        QString status = purchase["status"].toString();
        QString purchasedAt = purchase["purchasedAt"].toString();

        totalSpentSum += finalPrice;

        // کارت مربوط به هر سفارش
        QFrame* orderFrame = new QFrame();
        orderFrame->setStyleSheet(
            "QFrame {"
            "  background-color: #ffffff;"
            "  border: 2px solid #2b2b2b;"
            "  border-radius: 10px;"
            "}"
            "QFrame:hover {"
            "  background-color: #f0f7ff;"
            "  border: 2px solid #1e88e5;"
            "}"
            );
        orderFrame->setCursor(Qt::PointingHandCursor);
        orderFrame->setProperty("purchaseId", purchaseId);
        orderFrame->installEventFilter(this); // قابلیت کلیک روی کل ردیف

        QHBoxLayout* layout = new QHBoxLayout(orderFrame);
        layout->setContentsMargins(15, 10, 15, 10);

        // اطلاعات سفارش
        QLabel* infoLabel = new QLabel(
            QString("🛒 <b>Order #%1</b> &nbsp;&nbsp;|&nbsp;&nbsp; 📅 %2 &nbsp;&nbsp;|&nbsp;&nbsp; 📦 Items: <b>%3</b> &nbsp;&nbsp;|&nbsp;&nbsp; 💵 Total: <b>%4 T</b>")
                .arg(purchaseId)
                .arg(purchasedAt)
                .arg(totalItems)
                .arg(finalPrice)
            );
        infoLabel->setStyleSheet("font-size: 16px; color: black; border: none; background: transparent;");

        // وضعیت سفارش
        QLabel* statusLabel = new QLabel(QString("Status: <b>%1</b>").arg(status));
        statusLabel->setStyleSheet("font-size: 15px; color: #2e7d32; border: none; background: transparent;");

        // دکمه مشاهده جزئیات
        QPushButton* detailsBtn = new QPushButton("👁️ Details");
        detailsBtn->setFixedSize(110, 35);
        detailsBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #1e88e5;"
            "  color: white;"
            "  border-radius: 6px;"
            "  font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "  background-color: #1565c0;"
            "}"
            );
        detailsBtn->setProperty("purchaseId", purchaseId);
        connect(detailsBtn, &QPushButton::clicked, this, &ShoppingHistoryWindow::onOrderDetailsClicked);

        layout->addWidget(infoLabel);
        layout->addStretch();
        layout->addWidget(statusLabel);
        layout->addWidget(detailsBtn);

        ui->ordersLayout->addWidget(orderFrame);
    }

    ui->ordersLayout->addStretch();

    // 🟢 به‌روزرسانی کل مبلغ خرج‌شده در بالای صفحه
    ui->totalSpentValueLabel->setText(QString("%1 T").arg(totalSpentSum));
}

// 🟢 کلیک روی دکمه جزئیات سفارش
void ShoppingHistoryWindow::onOrderDetailsClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int purchaseId = button->property("purchaseId").toInt();

    QVariantMap params;
    params["purchaseId"] = purchaseId;

    // ارسال درخواست دریافت جزئیات اقلام سفارش به سرور
    Request request(CommandType::GetPurchaseById, params);
    m_networkManager->sendRequest(request);
}

// 🟢 کلیک روی کارت سفارش با موس
bool ShoppingHistoryWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame* frame = qobject_cast<QFrame*>(watched);
        if (frame && frame->property("purchaseId").isValid()) {
            int purchaseId = frame->property("purchaseId").toInt();

            QVariantMap params;
            params["purchaseId"] = purchaseId;

            Request request(CommandType::GetPurchaseById, params);
            m_networkManager->sendRequest(request);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void ShoppingHistoryWindow::clearLayout()
{
    QLayoutItem *item;
    while ((item = ui->ordersLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ShoppingHistoryWindow::on_backButton_clicked()
{
    emit backButtonClicked();
}