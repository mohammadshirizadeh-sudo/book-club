

/*
#include "editbookswindow.h"
#include "Publishers/ui_editbookswindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QPixmap>
#include <QListWidgetItem>
#include <QFileInfo>

EditBooksWindow::EditBooksWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditBooksWindow),
    m_selectedBookId(-1),
    m_hasUnsavedChanges(false)
{
    ui->setupUi(this);

    // Load initial books list
    loadBooksList();

    // Initially disable edit form
    enableEditMode(false);
}

EditBooksWindow::~EditBooksWindow()
{
    delete ui;
}

void EditBooksWindow::loadBooksList(const QString &searchText)
{
    // Get all books from database
    QList<BookData> allBooks = getPublisherBooksFromDB();

    // Apply search filter
    QList<BookData> filteredBooks = searchBooks(allBooks, searchText);

    // Clear and populate list
    ui->booksListWidget->clear();

    for (const BookData &book : filteredBooks) {
        QListWidgetItem *item = new QListWidgetItem(ui->booksListWidget);

        // Create rich text item
        QString displayText = QString(
                                  "<b>%1</b><br>"
                                  "<span style='color: #666; font-size: 10pt;'>by %2 | $%3 | %4</span>"
                                  ).arg(book.title)
                                  .arg(book.author)
                                  .arg(book.price, 0, 'f', 2)
                                  .arg(book.genre);

        item->setText(displayText);
        item->setData(Qt::UserRole, book.id);

        // Set icon based on cover availability
        if (!book.coverPath.isEmpty()) {
            item->setIcon(QIcon(book.coverPath));
        } else {
            item->setText("📖 " + displayText);  // Fallback emoji
        }

        ui->booksListWidget->addItem(item);
    }
}

void EditBooksWindow::populateEditForm(int bookId)
{
    if (bookId <= 0) {
        clearForm();
        return;
    }

    // Get book details from database
    BookData book = getBookDetails(bookId);

    // Store original values for reset functionality
    m_selectedBookId = book.id;
    m_originalCoverPath = book.coverPath;
    m_originalPdfPath = book.pdfPath;

    // Populate form fields
    ui->titleLineEdit->setText(book.title);
    ui->authorLineEdit->setText(book.author);

    // Find and set genre in combo box
    int genreIndex = ui->genreComboBox->findText(book.genre);
    if (genreIndex >= 0) {
        ui->genreComboBox->setCurrentIndex(genreIndex);
    } else {
        ui->genreComboBox->setCurrentIndex(0);  // Default to placeholder
    }

    ui->descriptionTextEdit->setPlainText(book.description);
    ui->priceSpinBox->setValue(book.price);
    ui->discountSpinBox->setValue(book.discount);

    // Update PDF path display
    if (book.pdfPath.isEmpty()) {
        ui->pdfPathLineEdit->setPlaceholderText("No PDF selected");
        ui->pdfPathLineEdit->clear();
    } else {
        QFileInfo fileInfo(book.pdfPath);
        ui->pdfPathLineEdit->setText(fileInfo.fileName());
    }

    // Update cover preview
    updateCoverPreview();

    // Enable edit mode
    enableEditMode(true);

    // Reset unsaved changes flag
    m_hasUnsavedChanges = false;

    updateStatusLabel(QString("Editing: \"%1\" - Changes will be saved when you click 'Save Changes'").arg(book.title));
}

void EditBooksWindow::enableEditMode(bool enable)
{
    ui->titleLineEdit->setEnabled(enable);
    ui->authorLineEdit->setEnabled(enable);
    ui->genreComboBox->setEnabled(enable);
    ui->descriptionTextEdit->setEnabled(enable);
    ui->priceSpinBox->setEnabled(enable);
    ui->discountSpinBox->setEnabled(enable);
    ui->uploadCoverButton->setEnabled(enable);
    ui->removeCoverButton->setEnabled(enable);
    ui->uploadPdfButton->setEnabled(enable);
    ui->saveChangesButton->setEnabled(enable);
    ui->resetFormButton->setEnabled(enable);

    // Show/hide no selection label
    ui->noSelectionLabel->setVisible(!enable);
}

void EditBooksWindow::clearForm()
{
    ui->titleLineEdit->clear();
    ui->authorLineEdit->clear();
    ui->genreComboBox->setCurrentIndex(0);
    ui->descriptionTextEdit->clear();
    ui->priceSpinBox->setValue(19.99);
    ui->discountSpinBox->setValue(0);
    ui->pdfPathLineEdit->clear();
    ui->pdfPathLineEdit->setPlaceholderText("No PDF selected");

    // Reset cover preview
    ui->coverTextLabel->setText("No image selected");
    m_coverPixmap = QPixmap();

    m_selectedBookId = -1;
    m_hasUnsavedChanges = false;

    enableEditMode(false);
    updateStatusLabel("Select a book from the list to begin editing.");
}

bool EditBooksWindow::validateInputs()
{
    // Validate title
    if (ui->titleLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Please enter a book title!");
        ui->titleLineEdit->setFocus();
        return false;
    }

    // Validate author
    if (ui->authorLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
                             "Please enter an author name!");
        ui->authorLineEdit->setFocus();
        return false;
    }

    // Validate genre
    if (ui->genreComboBox->currentIndex() == 0 ||
        ui->genreComboBox->currentText() == "-- Select Genre --") {
        QMessageBox::warning(this, "Validation Error",
                             "Please select a genre!");
        ui->genreComboBox->setFocus();
        return false;
    }

    // Validate price
    if (ui->priceSpinBox->value() < 0.99) {
        QMessageBox::warning(this, "Validation Error",
                             "Price must be at least $0.99!");
        ui->priceSpinBox->setFocus();
        return false;
    }

    return true;
}

bool EditBooksWindow::saveChangesToDatabase()
{
    if (!validateInputs()) {
        return false;
    }

    // Prepare data structure
    BookData updatedBook;
    updatedBook.id = m_selectedBookId;
    updatedBook.title = ui->titleLineEdit->text().trimmed();
    updatedBook.author = ui->authorLineEdit->text().trimmed();
    updatedBook.genre = ui->genreComboBox->currentText();
    updatedBook.description = ui->descriptionTextEdit->toPlainText().trimmed();
    updatedBook.price = ui->priceSpinBox->value();
    updatedBook.discount = ui->discountSpinBox->value();
    updatedBook.coverPath = m_originalCoverPath;  // Will be updated if changed
    updatedBook.pdfPath = m_originalPdfPath;      // Will be updated if changed

    // Save to database
    bool success = updateBookInDatabase(m_selectedBookId, updatedBook);

    if (success) {
        m_hasUnsavedChanges = false;
        updateStatusLabel("✅ Changes saved successfully!", "green");

        // Refresh books list to show updated info
        loadBooksList(ui->searchBookLineEdit->text());

        // Re-select current item in list
        for (int i = 0; i < ui->booksListWidget->count(); i++) {
            QListWidgetItem *item = ui->booksListWidget->item(i);
            if (item && item->data(Qt::UserRole).toInt() == m_selectedBookId) {
                ui->booksListWidget->setCurrentItem(item);
                break;
            }
        }
    }

    return success;
}

void EditBooksWindow::resetFormToOriginal()
{
    if (m_selectedBookId == -1) return;

    // Check for unsaved changes
    if (m_hasUnsavedChanges) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved Changes",
                                      "You have unsaved changes. Are you sure you want to reset?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }

    // Reload original data from database
    populateEditForm(m_selectedBookId);
    m_hasUnsavedChanges = false;
    updateStatusLabel("Form has been reset to original values.", "blue");
}

void EditBooksWindow::updateStatusLabel(const QString &message, const QString &color)
{
    ui->statusLabel->setText(message);
    if (color == "green") {
        ui->statusLabel->setStyleSheet("font: 10pt \"Segoe UI\"; color: rgb(56, 142, 60);");
    } else if (color == "red") {
        ui->statusLabel->setStyleSheet("font: 10pt \"Segoe UI\"; color: rgb(198, 40, 40);");
    } else if (color == "blue") {
        ui->statusLabel->setStyleSheet("font: 10pt \"Segoe UI\"; color: rgb(33, 150, 243);");
    } else {
        ui->statusLabel->setStyleSheet("font: 10pt \"Segoe UI\"; color: rgb(120, 120, 120);");
    }
}

void EditBooksWindow::updateCoverPreview()
{
    if (m_originalCoverPath.isEmpty()) {
        ui->coverTextLabel->setText("No image selected");
        ui->coverPlaceholderLabel->setVisible(true);
    } else {
        QPixmap pixmap(m_originalCoverPath);
        if (!pixmap.isNull()) {
            m_coverPixmap = pixmap.scaled(150, 190, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            // Note: In a real implementation, you would set this as a QLabel pixmap
            ui->coverTextLabel->setText("Image loaded");
            ui->coverPlaceholderLabel->setVisible(false);
        } else {
            ui->coverTextLabel->setText("Failed to load image");
        }
    }
}

// ==================== Database Methods (To be connected to real data source) ====================

QList<EditBooksWindow::BookData> EditBooksWindow::getPublisherBooksFromDB() const
{
    QList<BookData> books;

    // TODO: Connect to actual database
    // Sample data for demonstration
    BookData sampleBooks[] = {
        {1, "The Great Adventure", "John Smith", "Fiction",
         "An epic journey through uncharted territories filled with danger and discovery.",
         29.99, 15, "", "", "2024-01-15"},
        {2, "Mystery of Shadows", "Jane Doe", "Mystery / Thriller",
         "A gripping tale of secrets hidden in the darkness.",
         24.99, 10, "", "", "2024-02-20"},
        {3, "Science Today", "Dr. Alan Brown", "Science / Technology",
         "Exploring the latest breakthroughs in modern science.",
         34.99, 5, "", "", "2024-03-10"},
        {4, "Poetry Collection", "Emily White", "Poetry",
         "A beautiful collection of verses about life, love, and nature.",
         14.99, 0, "", "", "2023-11-05"},
        {5, "History Revealed", "Prof. Robert Lee", "History",
         "Uncovering the untold stories of ancient civilizations.",
         19.99, 8, "", "", "2024-04-22"},
        {6, "Advanced Mathematics", "Dr. Sarah Chen", "Educational",
         "Comprehensive guide to advanced mathematical concepts.",
         49.99, 12, "", "", "2024-05-18"}
    };

    for (int i = 0; i < 6; i++) {
        books.append(sampleBooks[i]);
    }

    return books;
}

QList<EditBooksWindow::BookData> EditBooksWindow::searchBooks(
    const QList<BookData> &books,
    const QString &searchText) const
{
    if (searchText.isEmpty()) {
        return books;
    }

    QList<BookData> result;
    QString searchLower = searchText.toLower();

    for (const BookData &book : books) {
        if (book.title.toLower().contains(searchLower) ||
            book.author.toLower().contains(searchLower) ||
            book.genre.toLower().contains(searchLower)) {
            result.append(book);
        }
    }

    return result;
}

EditBooksWindow::BookData EditBooksWindow::getBookDetails(int bookId) const
{
    Q_UNUSED(bookId)

    // TODO: Connect to actual database
    // Return sample data based on ID
    BookData sampleDetails = {1, "The Great Adventure", "John Smith", "Fiction",
                              "An epic journey through uncharted territories filled with danger and discovery.",
                              29.99, 15, "/path/to/cover.jpg", "/path/to/book.pdf", "2024-01-15"};

    // In real implementation, fetch from DB using bookId
    return sampleDetails;
}

bool EditBooksWindow::updateBookInDatabase(int bookId, const BookData &data)
{
    Q_UNUSED(bookId)
    Q_UNUSED(data)

    // TODO: Implement actual database update
    qDebug() << "Updating book:" << bookId << "-" << data.title;
    return true;
}

// ==================== Slot Implementations ====================
// Functionality slots
void EditBooksWindow::on_searchBookLineEdit_textChanged(const QString &text)
{
    loadBooksList(text);
}

void EditBooksWindow::on_refreshListBtn_clicked()
{
    loadBooksList(ui->searchBookLineEdit->text());
    updateStatusLabel("Book list refreshed!", "blue");
}

void EditBooksWindow::on_booksListWidget_currentRowChanged(int currentRow)
{
    if (currentRow < 0) {
        clearForm();
        return;
    }

    QListWidgetItem *item = ui->booksListWidget->currentItem();
    if (item) {
        int bookId = item->data(Qt::UserRole).toInt();
        populateEditForm(bookId);
    }
}

void EditBooksWindow::on_booksListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    // Double-click triggers same behavior as single click selection
    on_booksListWidget_currentRowChanged(ui->booksListWidget->currentRow());
}

void EditBooksWindow::on_titleLineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text)
    m_hasUnsavedChanges = true;
    updateStatusLabel("You have unsaved changes...", "orange");
}

void EditBooksWindow::on_authorLineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text)
    m_hasUnsavedChanges = true;
    updateStatusLabel("You have unsaved changes...", "orange");
}

void EditBooksWindow::on_genreComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    if (ui->genreComboBox->isEnabled()) {
        m_hasUnsavedChanges = true;
        updateStatusLabel("You have unsaved changes...", "orange");
    }
}

void EditBooksWindow::on_descriptionTextEdit_textChanged()
{
    if (ui->descriptionTextEdit->isEnabled()) {
        m_hasUnsavedChanges = true;
        updateStatusLabel("You have unsaved changes...", "orange");
    }
}

void EditBooksWindow::on_priceSpinBox_valueChanged(double value)
{
    Q_UNUSED(value)
    if (ui->priceSpinBox->isEnabled()) {
        m_hasUnsavedChanges = true;
        updateStatusLabel("You have unsaved changes...", "orange");
    }
}

void EditBooksWindow::on_discountSpinBox_valueChanged(int value)
{
    Q_UNUSED(value)
    if (ui->discountSpinBox->isEnabled()) {
        m_hasUnsavedChanges = true;
        updateStatusLabel("You have unsaved changes...", "orange");
    }
}

void EditBooksWindow::on_uploadCoverButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Cover Image",
        "",
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*)"
        );

    if (!fileName.isEmpty()) {
        m_originalCoverPath = fileName;
        updateCoverPreview();
        m_hasUnsavedChanges = true;
        updateStatusLabel("Cover image selected. Save to apply changes.", "blue");
    }
}

void EditBooksWindow::on_removeCoverButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove Cover Image",
                                  "Are you sure you want to remove the cover image?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_originalCoverPath.clear();
        m_coverPixmap = QPixmap();
        ui->coverTextLabel->setText("No image selected");
        ui->coverPlaceholderLabel->setVisible(true);
        m_hasUnsavedChanges = true;
        updateStatusLabel("Cover image removed. Save to apply changes.", "blue");
    }
}

void EditBooksWindow::on_uploadPdfButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select PDF File",
        "",
        "PDF Files (*.pdf);;All Files (*)"
        );

    if (!fileName.isEmpty()) {
        m_originalPdfPath = fileName;
        QFileInfo fileInfo(fileName);
        ui->pdfPathLineEdit->setText(fileInfo.fileName());
        m_hasUnsavedChanges = true;
        updateStatusLabel("PDF file selected. Save to apply changes.", "blue");
    }
}

void EditBooksWindow::on_saveChangesButton_clicked()
{
    if (saveChangesToDatabase()) {
        QMessageBox::information(this, "Success",
                                 "Book information has been updated successfully!\n\n"
                                 "Changes are now visible in the store.");
    } else {
        QMessageBox::critical(this, "Error",
                              "Failed to save changes. Please try again.");
    }
}

void EditBooksWindow::on_resetFormButton_clicked()
{
    resetFormToOriginal();
}
*/
