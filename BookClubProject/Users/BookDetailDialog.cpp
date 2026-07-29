#include"BookDetailDialog.h"
#include "Users/ui_BookDetailDialog.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../appWindow/SessionManager.h"
#include <QMessageBox>
#include <QPixmap>
#include "pdfreaderwindow.h"
#include "groupreadingwindow.h"
#include <QFile>

BookDetailDialog::BookDetailDialog(NetworkManager*networkManager , const QVariantMap& bookData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookDetailDialog),
    m_networkManager(networkManager),
    m_bookData(bookData)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &BookDetailDialog::onResponseReceived);


    displayBookInfo(bookData);
    checkBookOwnership();



}

BookDetailDialog::~BookDetailDialog()
{
    delete ui;
}

void BookDetailDialog::displayBookInfo(const QVariantMap& bookData)
{
    // پر کردن لِیبل‌ها بر اساس کلیدهایی که سرور پاس داده است
    ui->titleLabel->setText(bookData["title"].toString());
    ui->authorLabel->setText(bookData["author"].toString());
    ui->genreLabel->setText(bookData["genre"].toString());
    double price = bookData["price"].toDouble();
    double finalPrice = bookData["finalPrice"].toDouble();

    ui->priceLabel->setText(
        QString::number(price, 'f', 2) + " Tooman"
        );

    ui->finalPriceLabel->setText(
        QString::number(finalPrice, 'f', 2) + " Tooman"
        );
    ui->discountLabel->setText(bookData["discountPercent"].toString() + " ٪");
    ui->ratingLabel->setText(bookData["averageRating"].toString());
    int bookId = bookData["bookId"].toInt();
    bool isFav = bookData["isFavorite"].toBool();
    qDebug()<<"the bool is "<< isFav;
    m_isFavorite = isFav;

    QString coverPath = bookData["coverPath"].toString();
    if (!coverPath.isEmpty() && bookId > 0) {
        ui->coverLable->setAlignment(Qt::AlignCenter);
        ui->coverLable->setText("Loading Cover..."); // نمایش وضعیت بارگذاری به کاربر

        // 🟢 ارسال درخواست دریافت عکس به سرور (دقیقاً مشابه متد درون UserWindow)
        m_networkManager->requestBookCover(bookId);
    } else {
        ui->coverLable->setText("No Cover Image");
    }

    updateFavoriteButtonAppearance();
    updateCartButtonAppearance();
}

void BookDetailDialog::checkBookOwnership()
{
    int userId = SessionManager::instance()->getUserId();
    int bookId = m_bookData["bookId"].toInt();

    if (userId <= 0 || bookId <= 0) {
        return;
    }

    QVariantMap params;
    params["userId"] = userId;
    params["bookId"] = bookId;

    Request request(CommandType::CheckBookOwnership, params);
    m_networkManager->sendRequest(request);
}

void BookDetailDialog::updateCartButtonAppearance()
{
    if (m_isOwned || SessionManager::instance()->getRole() == "Admin") {
        ui->addCartPushButton->setText("📖 Open PDF");
        ui->addCartPushButton->setStyleSheet(
            "border: 3px solid black;"
            "border-radius: 12px;"
            "color: rgb(0, 0, 0);"
            "background-color: rgb(200, 255, 200);"
            "font: 700 9pt \"Script MT\";"
            "font-size: 40px;"
            );
    } else {
        ui->addCartPushButton->setText("Add to Cart");
        ui->addCartPushButton->setStyleSheet(
            "border: 3px solid black;"
            "border-radius: 12px;"
            "color: rgb(0, 0, 0);"
            "font: 700 9pt \"Script MT\";"
            "font-size: 40px;"
            );
    }
}

void BookDetailDialog::updateFavoriteButtonAppearance()
{
    if (m_isFavorite) {
        ui->addFavoritePushButton->setText("❤️ Remove from Favorites");
        ui->addFavoritePushButton->setStyleSheet("QPushButton { color: red; font-weight: bold; border: 2px solid red; border-radius: 8px; background-color: #fff0f0; }");
    } else {
        ui->addFavoritePushButton->setText("🤍 Add to Favorites");
        ui->addFavoritePushButton->setStyleSheet("QPushButton { color: black; font-weight: bold; border: 2px solid black; border-radius: 8px; background-color: white; }");
    }
}



void BookDetailDialog::onResponseReceived(const Response& response)
{
    // 🔴 توجه: نام Enum کامند دریافت عکس را بر اساس پروژه خود تنظیم کنید (مثلاً GetBookCover)
    if (response.getCommandType() == CommandType::GetBookCover) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            int responseBookId = resData["bookId"].toInt();
            int currentBookId = m_bookData["bookId"].toInt();

            // بررسی اینکه عکس دریافت شده دقیقاً متعلق به همین کتابِ باز شده باشد
            if (responseBookId == currentBookId) {
                // استخراج رشته متنی Base64 و تبدیل مجدد آن به دیتای باینری
                QString base64Data = resData["coverData"].toString();
                QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

                QPixmap pixmap;
                // لود کردن عکس از دیتای باینری دریافتی
                if (pixmap.loadFromData(imageData)) {
                    // اسکیل کردن عکس دقیقاً به اندازه خودِ لیبل با کیفیت بالا
                    QPixmap scaledPixmap = pixmap.scaled(ui->coverLable->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation);
                    ui->coverLable->setPixmap(scaledPixmap);
                    ui->coverLable->setText(""); // حذف متن Loading پس از موفقیت
                } else {
                    ui->coverLable->setText("Failed to process image data");
                }
            }
        } else {
            ui->coverLable->setText("Failed to download cover");
        }
    }else if (response.getCommandType() == CommandType::CheckBookOwnership) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            int responseBookId = data.value("bookId").toInt();
            int currentBookId = m_bookData["bookId"].toInt();

            // اطمینان از تعلق پاسخ به همین کتاب (در صورت باز بودن چند دیالوگ)
            if (responseBookId == currentBookId) {
                m_isOwned = data.value("isOwned").toBool();
                updateCartButtonAppearance();
            }
        }
        // در صورت خطا، فرض می‌کنیم مالک نیست و دکمه به شکل پیش‌فرض (Add to Cart) باقی می‌ماند
    }else if (response.getCommandType() == CommandType::AddFavoriteBook) {
        ui->addFavoritePushButton->setEnabled(true);

        if (response.isSuccess()) {
            m_isFavorite = true;
            updateFavoriteButtonAppearance();
            QMessageBox::information(this, "Favorites", "Book added to favorites successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Could not add to favorites: " + response.getMessage());
        }
    }else if (response.getCommandType() == CommandType::RemoveFavoriteBook) {
        ui->addFavoritePushButton->setEnabled(true); // فعال‌سازی مجدد دکمه

        if (response.isSuccess()) {
            m_isFavorite = false; // وضعیت به «حذف شده» تغییر میکند
            updateFavoriteButtonAppearance(); // تغییر قیافه دکمه به قلب سفید
            QMessageBox::information(this, "Favorites", "Book removed from favorites successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Could not remove from favorites: " + response.getMessage());
        }
    }else if (response.getCommandType() == CommandType::AddToCart) {
        // فعال کردن مجدد دکمه
        ui->addCartPushButton->setEnabled(true);
        ui->addCartPushButton->setText("🛒 Add to Cart");

        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            int totalItems = data.value("totalItems", 0).toInt();
            double finalPrice = data.value("finalPrice", 0.0).toDouble();

            QMessageBox::information(
                this,
                "Cart",
                QString("Book added to cart!\n\nTotal items: %1\nFinal price: $%2")
                    .arg(totalItems)
                    .arg(finalPrice, 0, 'f', 2)
                );
        } else {
            QMessageBox::warning(
                this,
                "Error",
                "Could not add to cart: " + response.getMessage()
                );
        }
        return;
    }
}


void BookDetailDialog::on_addFavoritePushButton_clicked()
{
    int userId = SessionManager::instance()->getUserId();
    int bookId = m_bookData["bookId"].toInt();

    if (userId <= 0 || bookId <= 0) {
        QMessageBox::warning(this, "Warning", "Invalid User or Book data.");
        return;
    }
    ui->addFavoritePushButton->setEnabled(false);

    QVariantMap params;
    params["userId"] = userId;
    params["bookId"] = bookId;

    if (m_isFavorite) {
        qDebug()<<"i go to isFav";

        Request request(CommandType::RemoveFavoriteBook, params);
        m_networkManager->sendRequest(request);
    } else {
        qDebug()<<"i go to else";
        Request request(CommandType::AddFavoriteBook, params);
        m_networkManager->sendRequest(request);
    }
}


void BookDetailDialog::on_addCartPushButton_clicked()
{
    if (m_isOwned) {
        openBookPdf();
        return;
    }

    if(SessionManager::instance()->getRole() == "Admin"){
        openBookPdf();
        return;
    }
    int userId = SessionManager::instance()->getUserId();

    // 2. دریافت bookId از اطلاعات کتاب
    int bookId = m_bookData["bookId"].toInt();

    // 3. اعتبارسنجی
    if (userId <= 0 || bookId <= 0) {
        QMessageBox::warning(this, "Warning", "Invalid User or Book data.");
        return;
    }

    // 4. غیرفعال کردن دکمه تا پاسخ بیاید
    ui->addCartPushButton->setEnabled(false);
    ui->addCartPushButton->setText("Adding...");

    // 5. ساخت پارامترها
    QVariantMap params;
    params["userId"] = userId;
    params["bookId"] = bookId;
    params["quantity"] = 1;  // تعداد پیش‌فرض 1

    // 6. ارسال درخواست به سرور
    Request request(CommandType::AddToCart, params);
    m_networkManager->sendRequest(request);
}

void BookDetailDialog::openBookPdf()
{
    int bookId = m_bookData["bookId"].toInt();
    QString title = m_bookData["title"].toString();
    QString filePath = m_bookData["pdfPath"].toString();

    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "خطا", "مسیر فایل PDF برای این کتاب ثبت نشده است.");
        return;
    }

    // ✅ بررسی وجود فایل قبل از هر کاری
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "خطا", "فایل PDF پیدا نشد:\n" + filePath);
        return;
    }

    PdfReaderWindow *readerWindow = new PdfReaderWindow(bookId);
    readerWindow->setAttribute(Qt::WA_DeleteOnClose);
    readerWindow->setWindowFlags(Qt::Window);
    readerWindow->setBookTitle(title);

    // ✅ اتصال سیگنال قبل از hide
    connect(readerWindow, &PdfReaderWindow::backRequested, this, [this, readerWindow]() {
        readerWindow->close();
        this->show();
    });

    // ✅ بارگذاری PDF
    if (readerWindow->loadPdf(filePath)) {
        this->hide();  // فقط در صورت موفقیت hide کن
        readerWindow->showMaximized();
        readerWindow->raise();
        readerWindow->activateWindow();
    } else {
        // ❌ در صورت خطا، پنجره PDF رو ببند و به کاربر خطا نشون بده
        QMessageBox::warning(this, "خطا", "بارگذاری فایل PDF با شکست مواجه شد.");
        readerWindow->deleteLater();  // یا delete readerWindow;
        // this->show() نیازی نیست چون hide نشده
    }
}
void BookDetailDialog::on_pushButton_clicked()
{
    openBookPdf();
}

// ─── Group Reading Button Handler ──────────────────────────────────────────────
// The button opens the Group Reading page directly from this dialog, so the
// action works no matter where the dialog was launched from. Previously this
// emitted `groupReadingRequested` and relied on each caller (UserWindow's
// free/recommended/new/best-seller click handlers) to re-emit it, which was
// only wired up for some of them and silently did nothing for the rest.
void BookDetailDialog::on_groupReadingPushButton_clicked()
{
    openGroupReading();
}

// Opens a dedicated GroupReadingWindow for the book currently shown in this
// dialog. Mirrors the openBookPdf() pattern: the window is heap-allocated,
// deletes itself on close, and is shown as a top-level window. The dialog is
// only hidden so that all of its loaded state (favorite flag, ownership flag,
// cover image, network subscriptions) is preserved when the user comes back.
void BookDetailDialog::openGroupReading()
{
    // 1. Validate the book before doing anything network-related.
    int bookId = m_bookData.value("bookId").toInt();
    if (bookId <= 0) {
        QMessageBox::warning(this,
                             tr("Group Reading"),
                             tr("Invalid book data. Cannot start a Group Reading session."));
        return;
    }

    // 2. Group Reading needs a PDF to render — GroupReadingWindow::loadBookPdf()
    //    will fall back to a placeholder message if pdfPath is empty, but it's
    //    a better UX to fail fast here with an actionable message instead of
    //    opening a window that immediately says "no readable file attached".
    QString pdfPath = m_bookData.value("pdfPath").toString();
    if (pdfPath.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Group Reading"),
                             tr("This book does not have a PDF available for Group Reading."));
        return;
    }

    // 3. Don't open a second Group Reading window if one is already on screen
    //    for this dialog (e.g. user alt-tabbed back to the dialog and clicked
    //    the button again). Just raise the existing one.
    if (m_groupReadingWindow) {
        m_groupReadingWindow->showMaximized();
        m_groupReadingWindow->raise();
        m_groupReadingWindow->activateWindow();
        return;
    }

    // 4. Create the window. WA_DeleteOnClose ensures the heap object is freed
    //    when the user closes it (via the back button or the OS close button).
    GroupReadingWindow *groupWindow = new GroupReadingWindow(m_networkManager);
    m_groupReadingWindow = groupWindow;
    groupWindow->setAttribute(Qt::WA_DeleteOnClose);
    groupWindow->setWindowFlags(Qt::Window);

    // 5. When the window is destroyed (either by WA_DeleteOnClose firing or
    //    by the parent dialog being torn down), clear our bookkeeping pointer.
    //    Using destroyed() rather than backRequested() means this also covers
    //    the case where the user closes the window via the title bar / Alt+F4.
    connect(groupWindow, &QObject::destroyed, this, [this](QObject*) {
        m_groupReadingWindow = nullptr;
    });

    // 6. Back button inside the Group Reading window: close it and bring the
    //    dialog back to the foreground. The dialog was only hidden, not
    //    closed, so all its state is intact.
    connect(groupWindow, &GroupReadingWindow::backRequested,
            this, [this]() {
                if (m_groupReadingWindow) {
                    m_groupReadingWindow->close();  // triggers WA_DeleteOnClose
                    m_groupReadingWindow = nullptr;
                }
                this->show();
                this->raise();
                this->activateWindow();
            });

    // 7. setBookData loads the cover, loads the PDF, and updates the title
    //    label. Passing sessionId=-1 means the user lands in the "create or
    //    join a session" state rather than auto-joining a non-existent
    //    session.
    groupWindow->setBookData(m_bookData, /*sessionId=*/-1);

    // 8. Hide the dialog and show the Group Reading window maximized, the
    //    same way openBookPdf() presents the PdfReaderWindow.
    this->hide();
    groupWindow->showMaximized();
    groupWindow->raise();
    groupWindow->activateWindow();
}
