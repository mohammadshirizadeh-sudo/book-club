#include "deactivatebookwindow.h"
#include "Publishers/ui_deactivatebookwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QTextStream>
#include <QTableWidgetItem>

DeactivateBookWindow::DeactivateBookWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeactivateBookWindow),
    m_selectedBookId(-1),
    m_isSelectedBookActive(false)
{
    ui->setupUi(this);

    // Load initial books list
    loadBooksList();

    // Configure table selection behavior
    ui->booksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->booksTable->setSelectionMode(QAbstractItemView::SingleSelection);
}

DeactivateBookWindow::~DeactivateBookWindow()
{
    delete ui;
}

void DeactivateBookWindow::loadBooksList(const QString &filter, const QString &searchText)
{
    // Get all books from database
    QList<BookData> allBooks = getPublisherBooksFromDB();

    // Apply filters
    QList<BookData> filteredBooks = filterBooks(allBooks, filter, searchText);

    // Clear and populate table
    ui->booksTable->setRowCount(0);

    for (int i = 0; i < filteredBooks.size(); i++) {
        const BookData &book = filteredBooks[i];
        int row = i;
        ui->booksTable->insertRow(row);

        // ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(book.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setFont(QFont("Segoe UI", 9));
        ui->booksTable->setItem(row, 0, idItem);

        // Cover placeholder
        QTableWidgetItem *coverItem = new QTableWidgetItem("📖");
        coverItem->setTextAlignment(Qt::AlignCenter);
        coverItem->setFont(QFont("Segoe UI", 16));
        coverItem->setBackground(QColor(248, 248, 248));
        ui->booksTable->setItem(row, 1, coverItem);

        // Title
        QTableWidgetItem *titleItem = new QTableWidgetItem(book.title);
        titleItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->booksTable->setItem(row, 2, titleItem);

        // Author
        QTableWidgetItem *authorItem = new QTableWidgetItem(book.author);
        authorItem->setTextAlignment(Qt::AlignCenter);
        ui->booksTable->setItem(row, 3, authorItem);

        // Price
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString("$%1").arg(book.price, 0, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignCenter);
        priceItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        priceItem->setForeground(QColor(33, 150, 243));
        ui->booksTable->setItem(row, 4, priceItem);

        // Status with colored badge
        QTableWidgetItem *statusItem = new QTableWidgetItem();
        if (book.status == "active") {
            statusItem->setText("✅ Active");
            statusItem->setForeground(QColor(56, 142, 60));
            statusItem->setBackground(QColor(232, 245, 233));
        } else {
            statusItem->setText("🚫 Inactive");
            statusItem->setForeground(QColor(198, 40, 40));
            statusItem->setBackground(QColor(255, 235, 238));
        }
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        ui->booksTable->setItem(row, 5, statusItem);

        // Sales
        QTableWidgetItem *salesItem = new QTableWidgetItem(QString::number(book.sales));
        salesItem->setTextAlignment(Qt::AlignCenter);
        ui->booksTable->setItem(row, 6, salesItem);

        // Publish Date
        QTableWidgetItem *dateItem = new QTableWidgetItem(book.publishDate);
        dateItem->setTextAlignment(Qt::AlignCenter);
        ui->booksTable->setItem(row, 7, dateItem);

        // Actions - show appropriate button text
        QString actionText = (book.status == "active") ? "🚫 Deactivate" : "✅ Reactivate";
        QTableWidgetItem *actionItem = new QTableWidgetItem(actionText);
        actionItem->setTextAlignment(Qt::AlignCenter);
        actionItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        if (book.status == "active") {
            actionItem->setForeground(QColor(255, 87, 34));
        } else {
            actionItem->setForeground(QColor(76, 175, 80));
        }
        ui->booksTable->setItem(row, 8, actionItem);

        // Store book ID in first column for retrieval
        idItem->setData(Qt::UserRole, book.id);
        idItem->setData(Qt::UserRole + 1, book.status);
    }

    // Update statistics label
    int activeCount = 0;
    int deactivatedCount = 0;
    for (const BookData &book : filteredBooks) {
        if (book.status == "active") activeCount++;
        else deactivatedCount++;
    }

    ui->tableTitleLabel->setText(
        QString("📋 Your Books Inventory | Showing: %1 total (%2 active, %3 deactivated)")
            .arg(filteredBooks.size())
            .arg(activeCount)
            .arg(deactivatedCount)
        );
}

void DeactivateBookWindow::updateSelectionState(int row)
{
    if (row < 0 || row >= ui->booksTable->rowCount()) {
        m_selectedBookId = -1;
        m_selectedBookTitle.clear();
        m_isSelectedBookActive = false;
        updateActionButtons();
        return;
    }

    // Get book ID from user role data
    QTableWidgetItem *idItem = ui->booksTable->item(row, 0);
    if (idItem) {
        m_selectedBookId = idItem->data(Qt::UserRole).toInt();
        m_isSelectedBookActive = (idItem->data(Qt::UserRole + 1).toString() == "active");

        // Get title
        QTableWidgetItem *titleItem = ui->booksTable->item(row, 2);
        m_selectedBookTitle = titleItem ? titleItem->text() : "";
    }

    // Update selected book label
    ui->selectedBookLabel->setText(
        QString("Selected: %1 (ID: %2) - Status: %3")
            .arg(m_selectedBookTitle)
            .arg(m_selectedBookId)
            .arg(m_isSelectedBookActive ? "✅ Active" : "🚫 Inactive")
        );

    updateActionButtons();
}

void DeactivateBookWindow::updateActionButtons()
{
    bool hasSelection = (m_selectedBookId != -1);

    ui->deactivateBookButton->setEnabled(hasSelection && m_isSelectedBookActive);
    ui->reactivateBookButton->setEnabled(hasSelection && !m_isSelectedBookActive);
    ui->deleteBookButton->setEnabled(hasSelection);
    ui->viewDetailsButton->setEnabled(hasSelection);
}

void DeactivateBookWindow::deactivateSelectedBook()
{
    if (m_selectedBookId == -1) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Deactivate Book",
        QString("Are you sure you want to deactivate \"%1\"?\n\n"
                "The book will be removed from:\n"
                "• Store listings\n"
                "• Search results\n\n"
                "Note: Buyers who already purchased this book can still access it in their library.")
            .arg(m_selectedBookTitle),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        bool success = deactivateBookInDatabase(m_selectedBookId);

        if (success) {
            QMessageBox::information(this, "Success",
                                     QString("\"%1\" has been successfully deactivated.\n"
                                             "It is no longer visible in the store.").arg(m_selectedBookTitle));

            // Refresh list
            loadBooksList(getCurrentFilter(), ui->searchLineEdit->text());
            m_selectedBookId = -1;
            updateActionButtons();
        } else {
            QMessageBox::critical(this, "Error",
                                  "Failed to deactivate the book. Please try again.");
        }
    }
}

void DeactivateBookWindow::reactivateSelectedBook()
{
    if (m_selectedBookId == -1) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Reactivate Book",
        QString("Are you sure you want to reactivate \"%1\"?\n\n"
                "The book will become visible again in:")
                .arg(m_selectedBookTitle) +
            "\n• Store listings\n• Search results",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        bool success = reactivateBookInDatabase(m_selectedBookId);

        if (success) {
            QMessageBox::information(this, "Success",
                                     QString("\"%1\" has been successfully reactivated!\n"
                                             "It is now visible in the store again.").arg(m_selectedBookTitle));

            // Refresh list
            loadBooksList(getCurrentFilter(), ui->searchLineEdit->text());
            m_selectedBookId = -1;
            updateActionButtons();
        } else {
            QMessageBox::critical(this, "Error",
                                  "Failed to reactivate the book. Please try again.");
        }
    }
}

void DeactivateBookWindow::deleteSelectedBook()
{
    if (m_selectedBookId == -1) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(
        this,
        "⚠️ PERMANENT DELETION WARNING",
        QString("⚠️ WARNING: You are about to PERMANENTLY DELETE \"%1\"!\n\n"
                "This action CANNOT be undone! The following data will be lost:\n\n"
                "📖 Book information and content\n"
                "💰 All sales records for this book\n"
                "👥 Customer purchase history\n"
                "⭐ Reviews and ratings\n"
                "🏷️ Any active discounts\n\n"
                "Are you absolutely sure you want to proceed?")
            .arg(m_selectedBookTitle),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No  // Default to No for safety
        );

    if (reply == QMessageBox::Yes) {
        // Second confirmation for extra safety
        QMessageBox::StandardButton finalConfirm;
        finalConfirm = QMessageBox::critical(
            this,
            "FINAL CONFIRMATION",
            QString("LAST CHANCE: This is your FINAL opportunity to cancel.\n\n"
                    "Deleting \"%1\" permanently...\n\n"
                    "Type 'DELETE' to confirm or press Cancel to abort.")
                .arg(m_selectedBookTitle),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel
            );

        if (finalConfirm == QMessageBox::Ok) {
            bool success = deleteBookPermanently(m_selectedBookId);

            if (success) {
                QMessageBox::information(this, "Deleted",
                                         QString("\"%1\" has been permanently deleted from the system.\n"
                                                 "All associated data has been removed.").arg(m_selectedBookTitle));

                // Refresh list
                loadBooksList(getCurrentFilter(), ui->searchLineEdit->text());
                m_selectedBookId = -1;
                updateActionButtons();
            } else {
                QMessageBox::critical(this, "Error",
                                      "Failed to delete the book. Please try again.");
            }
        }
    }
}

void DeactivateBookWindow::viewBookDetails()
{
    if (m_selectedBookId == -1) return;

    // TODO: Open detailed view dialog
    QMessageBox::information(this,
                             "Book Details",
                             QString("Detailed view for \"%1\"\n\n"
                                     "This would open a comprehensive details dialog showing:\n"
                                     "• Full book information\n"
                                     "• Sales analytics\n"
                                     "• Customer reviews\n"
                                     "• Purchase history").arg(m_selectedBookTitle));
}

void DeactivateBookWindow::exportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Books List",
        "books_export.csv",
        "CSV Files (*.csv);;All Files (*)"
        );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not create file for export.");
        return;
    }

    QTextStream out(&file);

    // Write header
    out << "ID,Title,Author,Price,Status,Sales,Publish Date\n";

    // Write data rows
    for (int row = 0; row < ui->booksTable->rowCount(); row++) {
        QStringList rowData;
        for (int col = 0; col < 8; col++) {  // Exclude actions column
            QTableWidgetItem *item = ui->booksTable->item(row, col);
            if (item) {
                QString text = item->text();
                // Escape commas in CSV
                if (text.contains(',')) {
                    text = "\"" + text + "\"";
                }
                rowData << text;
            } else {
                rowData << "";
            }
        }
        out << rowData.join(",") << "\n";
    }

    file.close();

    QMessageBox::information(this, "Export Complete",
                             QString("Successfully exported %1 books to:\n%2")
                                 .arg(ui->booksTable->rowCount())
                                 .arg(fileName));
}

QString DeactivateBookWindow::getCurrentFilter() const
{
    if (ui->allBooksRadio->isChecked()) return "all";
    if (ui->activeBooksRadio->isChecked()) return "active";
    if (ui->deactivatedBooksRadio->isChecked()) return "deactivated";
    return "all";
}

// ==================== Database Methods (To be connected to real data source) ====================

QList<DeactivateBookWindow::BookData> DeactivateBookWindow::getPublisherBooksFromDB() const
{
    QList<BookData> books;

    // TODO: Connect to actual database
    // Sample data for demonstration
    BookData sampleBooks[] = {
        {1, "The Great Adventure", "John Smith", 29.99, "active", 150, "2024-01-15", ""},
        {2, "Mystery of Shadows", "Jane Doe", 24.99, "active", 132, "2024-02-20", ""},
        {3, "Science Today", "Dr. Alan Brown", 34.99, "active", 98, "2024-03-10", ""},
        {4, "Poetry Collection", "Emily White", 14.99, "deactivated", 87, "2023-11-05", ""},
        {5, "History Revealed", "Prof. Robert Lee", 19.99, "active", 76, "2024-04-22", ""},
        {6, "Advanced Mathematics", "Dr. Sarah Chen", 49.99, "active", 8, "2024-05-18", ""},
        {7, "Rare Botany Guide", "Dr. Maria Garcia", 39.99, "deactivated", 12, "2023-09-30", ""},
        {8, "Local History Vol.3", "James Wilson", 25.00, "active", 15, "2024-06-01", ""}
    };

    for (int i = 0; i < 8; i++) {
        books.append(sampleBooks[i]);
    }

    return books;
}

QList<DeactivateBookWindow::BookData> DeactivateBookWindow::filterBooks(
    const QList<BookData> &books,
    const QString &statusFilter,
    const QString &searchText) const
{
    QList<BookData> result;

    for (const BookData &book : books) {
        // Apply status filter
        if (statusFilter != "all" && book.status != statusFilter) {
            continue;
        }

        // Apply search filter
        if (!searchText.isEmpty()) {
            QString searchLower = searchText.toLower();
            if (!book.title.toLower().contains(searchLower) &&
                !book.author.toLower().contains(searchLower)) {
                continue;
            }
        }

        result.append(book);
    }

    return result;
}

bool DeactivateBookWindow::deactivateBookInDatabase(int bookId)
{
    Q_UNUSED(bookId)
    // TODO: Implement actual database operation
    qDebug() << "Deactivating book:" << bookId;
    return true;
}

bool DeactivateBookWindow::reactivateBookInDatabase(int bookId)
{
    Q_UNUSED(bookId)
    // TODO: Implement actual database operation
    qDebug() << "Reactivating book:" << bookId;
    return true;
}

bool DeactivateBookWindow::deleteBookPermanently(int bookId)
{
    Q_UNUSED(bookId)
    // TODO: Implement actual database operation
    qDebug() << "Permanently deleting book:" << bookId;
    return true;
}

// ==================== Slot Implementations ====================
// Functionality slots
void DeactivateBookWindow::on_allBooksRadio_toggled(bool checked)
{
    if (checked) {
        loadBooksList("all", ui->searchLineEdit->text());
    }
}

void DeactivateBookWindow::on_activeBooksRadio_toggled(bool checked)
{
    if (checked) {
        loadBooksList("active", ui->searchLineEdit->text());
    }
}

void DeactivateBookWindow::on_deactivatedBooksRadio_toggled(bool checked)
{
    if (checked) {
        loadBooksList("deactivated", ui->searchLineEdit->text());
    }
}

void DeactivateBookWindow::on_searchLineEdit_textChanged(const QString &text)
{
    loadBooksList(getCurrentFilter(), text);
}

void DeactivateBookWindow::on_refreshListButton_clicked()
{
    loadBooksList(getCurrentFilter(), ui->searchLineEdit->text());
    QMessageBox::information(this, "Refreshed", "Book list has been refreshed!");
}

void DeactivateBookWindow::on_exportListButton_clicked()
{
    exportToCSV();
}

void DeactivateBookWindow::on_booksTable_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    updateSelectionState(row);
}

void DeactivateBookWindow::on_booksTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    updateSelectionState(row);
    viewBookDetails();
}

void DeactivateBookWindow::on_deactivateBookButton_clicked()
{
    deactivateSelectedBook();
}

void DeactivateBookWindow::on_reactivateBookButton_clicked()
{
    reactivateSelectedBook();
}

void DeactivateBookWindow::on_deleteBookButton_clicked()
{
    deleteSelectedBook();
}

void DeactivateBookWindow::on_viewDetailsButton_clicked()
{
    viewBookDetails();
}
