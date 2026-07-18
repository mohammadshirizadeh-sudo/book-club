#include "shelfmanagementdialog.h"
#include "Users/ui_shelfmanagementdialog.h"

#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Shared/Book.h"
#include "../Shared/Shelf.h"

#include <QMessageBox>
#include <QListWidgetItem>
#include <QInputDialog>
#include <QPixmap>

ShelfManagementDialog::ShelfManagementDialog(NetworkManager* networkManager, int userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ShelfManagementDialog)
    , m_networkManager(networkManager)
    , m_userId(userId)
    , m_currentShelfId(-1)
    , m_currentBookId(-1)
{
    ui->setupUi(this);

    // Connect network manager response signal
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &ShelfManagementDialog::onResponseReceived);

    // Load initial data
    loadShelves();
}

ShelfManagementDialog::~ShelfManagementDialog()
{
    delete ui;
}

void ShelfManagementDialog::loadShelves()
{
    QVariantMap params;
    params["userId"] = m_userId;

    Request request(CommandType::GetUserShelves, params);

    m_networkManager->sendRequest(request);
}

void ShelfManagementDialog::loadBooksForShelf(int shelfId)
{
    if (shelfId < 0) return;

    m_currentShelfId = shelfId;

    QVariantMap params;
    params["shelfId"] = shelfId;

    Request request(CommandType::GetBooksInShelf, params);
    m_networkManager->sendRequest(request);
}

void ShelfManagementDialog::updateBookDetails(int bookId)
{
    if (bookId < 0) {
        clearBookDetails();
        return;
    }

    m_currentBookId = bookId;

    QVariantMap params;
    params["bookId"] = bookId;

    Request request(CommandType::GetBookById, params);
    m_networkManager->sendRequest(request);

    // Enable buttons
    ui->removeBookButton->setEnabled(true);
    ui->moveBookButton->setEnabled(true);
    ui->readPdfButton->setEnabled(true);
}

void ShelfManagementDialog::clearBookDetails()
{
    m_currentBookId = -1;

    ui->coverPreviewLabel->setText("No book selected");
    ui->coverPreviewLabel->setPixmap(QPixmap());
    ui->bookTitleValueLabel->setText("Select a book to view details");
    ui->authorValueLabel->setText("");
    ui->genreValueLabel->setText("");
    ui->priceValueLabel->setText("");
    ui->descriptionTextEdit->clear();

    // Disable buttons
    ui->removeBookButton->setEnabled(false);
    ui->moveBookButton->setEnabled(false);
    ui->readPdfButton->setEnabled(false);
}

int ShelfManagementDialog::getSelectedShelfId() const
{
    QListWidgetItem *currentItem = ui->shelfList->currentItem();
    if (currentItem) {
        return currentItem->data(Qt::UserRole).toInt();
    }
    return -1;
}

int ShelfManagementDialog::getSelectedBookId() const
{
    QListWidgetItem *currentItem = ui->bookList->currentItem();
    if (currentItem) {
        return currentItem->data(Qt::UserRole).toInt();
    }
    return -1;
}

// ===== Slot Implementations =====

void ShelfManagementDialog::on_addShelfButton_clicked()
{
    bool ok;
    QString shelfName = QInputDialog::getText(
        this, "Create New Shelf",
        "Enter shelf name:",
        QLineEdit::Normal,
        "", &ok
        );

    if (ok && !shelfName.isEmpty()) {
        QVariantMap params;
        params["userId"] = m_userId;
        params["shelfName"] = shelfName;

        Request request(CommandType::CreateShelf, params);
        m_networkManager->sendRequest(request);
    }
}

void ShelfManagementDialog::on_deleteShelfButton_clicked()
{
    int shelfId = getSelectedShelfId();
    if (shelfId < 0) {
        QMessageBox::information(this, "Info", "Please select a shelf to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        "Are you sure you want to delete this shelf?\nAll books in this shelf will be removed.",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QVariantMap params;
        params["shelfId"] = shelfId;

        Request request(CommandType::DeleteShelf, params);

        m_networkManager->sendRequest(request);
    }
}

void ShelfManagementDialog::on_renameShelfButton_clicked()
{
    int shelfId = getSelectedShelfId();
    if (shelfId < 0) {
        QMessageBox::information(this, "Info", "Please select a shelf to rename.");
        return;
    }

    QListWidgetItem *currentItem = ui->shelfList->currentItem();
    QString currentName = currentItem ? currentItem->text() : "";

    bool ok;
    QString newName = QInputDialog::getText(
        this, "Rename Shelf",
        "Enter new shelf name:",
        QLineEdit::Normal,
        currentName, &ok
        );

    if (ok && !newName.isEmpty() && newName != currentName) {
        QVariantMap params;
        params["shelfId"] = shelfId;
        params["newName"] = newName;

        Request request(CommandType::RenameShelf, params);
        m_networkManager->sendRequest(request);
    }
}

void ShelfManagementDialog::on_removeBookButton_clicked()
{
    int bookId = getSelectedBookId();
    int shelfId = getSelectedShelfId();

    if (bookId < 0 || shelfId < 0) {
        QMessageBox::information(this, "Info", "Please select a book and shelf.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Remove",
        "Remove this book from the current shelf?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QVariantMap params;
        params["shelfId"] = shelfId;
        params["bookId"] = bookId;

        Request request(CommandType::RemoveBookFromShelf, params);
        m_networkManager->sendRequest(request);
    }
}

void ShelfManagementDialog::on_moveBookButton_clicked()
{
    int bookId = getSelectedBookId();
    int fromShelfId = getSelectedShelfId();

    if (bookId < 0 || fromShelfId < 0) {
        QMessageBox::information(this, "Info", "Please select a book first.");
        return;
    }

    // Get list of shelves for selection
    QStringList shelfNames;
    QList<int> shelfIds;

    for (int i = 0; i < ui->shelfList->count(); ++i) {
        QListWidgetItem *item = ui->shelfList->item(i);
        if (item && item->data(Qt::UserRole).toInt() != fromShelfId) {
            shelfNames << item->text();
            shelfIds << item->data(Qt::UserRole).toInt();
        }
    }

    if (shelfNames.isEmpty()) {
        QMessageBox::information(this, "Info", "No other shelves available.");
        return;
    }

    bool ok;
    QString selectedShelf = QInputDialog::getItem(
        this, "Move Book",
        "Select destination shelf:",
        shelfNames, 0, false, &ok
        );

    if (ok && !selectedShelf.isEmpty()) {
        int destShelfIndex = shelfNames.indexOf(selectedShelf);
        if (destShelfIndex >= 0) {
            int destShelfId = shelfIds[destShelfIndex];

            QVariantMap params;
            params["fromShelfId"] = fromShelfId;
            params["toShelfId"] = destShelfId;
            params["bookId"] = bookId;

            Request request(CommandType::MoveBookBetweenShelves, params);
            m_networkManager->sendRequest(request);
        }
    }
}

void ShelfManagementDialog::on_readPdfButton_clicked()
{
    int bookId = getSelectedBookId();
    if (bookId > 0) {
        emit openPdfReader(bookId);
    }
}

void ShelfManagementDialog::on_shelfList_currentRowChanged(int currentRow)
{
    Q_UNUSED(currentRow);
    int shelfId = getSelectedShelfId();
    loadBooksForShelf(shelfId);
}

void ShelfManagementDialog::on_bookList_currentItemChanged(QListWidgetItem *current)
{
    if (current) {
        int bookId = current->data(Qt::UserRole).toInt();
        updateBookDetails(bookId);
    } else {
        clearBookDetails();
    }
}

void ShelfManagementDialog::onResponseReceived(const Response& response)
{
    switch (response.getCommandType()) {
    case CommandType::GetUserShelves:
        if (response.isSuccess()) {
            ui->shelfList->clear();

            QVariantList shelves = response.getData()["shelves"].toList();
            for (const QVariant &shelfVar : shelves) {
                QVariantMap shelf = shelfVar.toMap();

                QListWidgetItem *item = new QListWidgetItem();
                item->setText(shelf["name"].toString());
                item->setData(Qt::UserRole, shelf["id"].toInt());
                ui->shelfList->addItem(item);
            }

            // Auto-select first shelf
            if (ui->shelfList->count() > 0) {
                ui->shelfList->setCurrentRow(0);
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to load shelves: " + response.getMessage());
        }
        break;

    case CommandType::GetBooksInShelf:
        if (response.isSuccess()) {
            ui->bookList->clear();

            QVariantList books = response.getData()["books"].toList();
            for (const QVariant &bookVar : books) {
                QVariantMap book = bookVar.toMap();

                QListWidgetItem *item = new QListWidgetItem();
                item->setText(book["title"].toString());
                item->setData(Qt::UserRole, book["id"].toInt());

                // Set icon if cover image available
                QPixmap coverPixmap;
                if (coverPixmap.loadFromData(QByteArray::fromBase64(book["coverImage"].toByteArray()))) {
                    QIcon coverIcon(coverPixmap.scaled(80, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    item->setIcon(coverIcon);
                }

                ui->bookList->addItem(item);
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to load books: " + response.getMessage());
        }
        break;

    case CommandType::GetBookById:
        if (response.isSuccess()) {
            QVariantMap book = response.getData()["book"].toMap();

            ui->bookTitleValueLabel->setText(book["title"].toString());
            ui->authorValueLabel->setText(QString("Author: %1").arg(book["author"].toString()));
            ui->genreValueLabel->setText(QString("Genre: %1").arg(book["genre"].toString()));
            ui->priceValueLabel->setText(QString("Price: $%1").arg(book["price"].toDouble(), 0, 'f', 2));
            ui->descriptionTextEdit->setPlainText(book["description"].toString());

            // Update cover image
            QByteArray coverData = QByteArray::fromBase64(book["coverImage"].toByteArray());
            QPixmap coverPixmap;
            if (!coverData.isEmpty() && coverPixmap.loadFromData(coverData)) {
                ui->coverPreviewLabel->setPixmap(coverPixmap.scaled(
                    ui->coverPreviewLabel->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    ));
                ui->coverPreviewLabel->setText("");
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to load book details: " + response.getMessage());
        }
        break;

    case CommandType::CreateShelf:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Shelf created successfully!");
            loadShelves();
        } else {
            QMessageBox::warning(this, "Error", "Failed to create shelf: " + response.getMessage());
        }
        break;

    case CommandType::DeleteShelf:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Shelf deleted successfully!");
            loadShelves();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete shelf: " + response.getMessage());
        }
        break;

    case CommandType::RenameShelf:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Shelf renamed successfully!");
            loadShelves();
        } else {
            QMessageBox::warning(this, "Error", "Failed to rename shelf: " + response.getMessage());
        }
        break;

    case CommandType::RemoveBookFromShelf:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Book removed from shelf!");
            loadBooksForShelf(m_currentShelfId);
        } else {
            QMessageBox::warning(this, "Error", "Failed to remove book: " + response.getMessage());
        }
        break;

    case CommandType::MoveBookBetweenShelves:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Book moved successfully!");
            loadBooksForShelf(m_currentShelfId);
        } else {
            QMessageBox::warning(this, "Error", "Failed to move book: " + response.getMessage());
        }
        break;

    default:
        break;
    }
}
