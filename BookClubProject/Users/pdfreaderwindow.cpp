

/*
#include "pdfreaderwindow.h"
#include "Users/ui_pdfreaderwindow.h"

// #include <QPdfDocument>
// #include <QPdfView>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QScrollBar>

PdfReaderWindow::PdfReaderWindow(int bookId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PdfReaderWindow)
    , m_pdfDocument(new QPdfDocument(this))
    , m_pdfView(nullptr)
    , m_bookId(bookId)
    , m_currentPage(0)
    , m_totalPages(0)
    , m_zoomFactor(1.0)
{
    ui->setupUi(this);

    // Setup PDF view
    setupPdfView();

    // Set default zoom
    ui->zoomCombo->setCurrentIndex(2); // 100%
}

PdfReaderWindow::~PdfReaderWindow()
{
    delete ui;
}

void PdfReaderWindow::setupPdfView()
{
    // Create PDF view widget inside container
    m_pdfView = new QPdfView(ui->pdfViewContainer);
    m_pdfView->setDocument(m_pdfDocument);
    m_pdfView->setGeometry(ui->pdfViewContainer->contentsRect());
    m_pdfView->setPageMode(QPdfView::PageMode::SinglePage);
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(1.0);

    // Hide placeholder label when PDF is loaded
    connect(m_pdfDocument, &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status status) {
        if (status == QPdfDocument::Ready) {
            ui->placeholderLabel->hide();
            m_totalPages = m_pdfDocument->pageCount();
            updatePageInfo();
        }
    });

    // Connect page navigation signal
    connect(m_pdfView->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        int newPage = m_pdfView->currentPage();
        if (newPage != m_currentPage) {
            m_currentPage = newPage;
            updatePageInfo();
            emit pageChanged(m_currentPage + 1);
        }
    });
}

bool PdfReaderWindow::loadPdf(const QString &filePath)
{
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No file path provided.");
        return false;
    }

    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "Error",
                             QString("PDF file not found:\n%1").arg(filePath));
        return false;
    }

    bool success = m_pdfDocument->load(filePath);

    if (success) {
        m_totalPages = m_pdfDocument->pageCount();
        m_currentPage = 0;
        updatePageInfo();
        ui->statusLabel->setText(QString("Loaded: %1").arg(QFileInfo(filePath).fileName()));

        // Show PDF view and hide placeholder
        m_pdfView->show();
        m_pdfView->setGeometry(ui->pdfViewContainer->contentsRect());
        ui->placeholderLabel->hide();
    } else {
        QMessageBox::critical(this, "Error",
                              QString("Failed to load PDF:\n%1").arg(filePath));
        ui->statusLabel->setText("Failed to load document");
    }

    return success;
}

void PdfReaderWindow::setBookTitle(const QString &title)
{
    ui->bookTitleLabel->setText(title);
}

void PdfReaderWindow::updatePageInfo()
{
    if (m_totalPages > 0) {
        ui->pageInfoLabel->setText(
            QString("Page %1 of %2").arg(m_currentPage + 1).arg(m_totalPages));
        ui->pageEdit->setText(QString::number(m_currentPage + 1));

        // Update progress
        double progress = (static_cast<double>(m_currentPage + 1) / m_totalPages) * 100.0;
        ui->positionLabel->setText(QString("Progress: %1%").arg(progress, 0, 'f', 1));
    } else {
        ui->pageInfoLabel->setText("No document loaded");
        ui->positionLabel->setText("Progress: 0.0%");
    }
}

void PdfReaderWindow::updateZoomControls()
{
    int zoomPercent = static_cast<int>(m_zoomFactor * 100.0);

    // Update combo box without triggering signal
    ui->zoomCombo->blockSignals(true);

    if (zoomPercent <= 50) ui->zoomCombo->setCurrentIndex(0);
    else if (zoomPercent <= 75) ui->zoomCombo->setCurrentIndex(1);
    else if (zoomPercent <= 125) ui->zoomCombo->setCurrentIndex(2);
    else if (zoomPercent <= 175) ui->zoomCombo->setCurrentIndex(3);
    else if (zoomPercent <= 225) ui->zoomCombo->setCurrentIndex(4);
    else if (zoomPercent <= 275) ui->zoomCombo->setCurrentIndex(5);
    else if (zoomPercent <= 375) ui->zoomCombo->setCurrentIndex(6);
    else ui->zoomCombo->setCurrentIndex(7);

    ui->zoomCombo->blockSignals(false);

    ui->zoomStatusLabel->setText(QString("Zoom: %1%").arg(zoomPercent));
}

void PdfReaderWindow::goToPage(int page)
{
    if (page < 0 || page >= m_totalPages) return;

    m_currentPage = page;
    m_pdfView->setCurrentPage(page);
    updatePageInfo();
    emit pageChanged(page + 1);
}

void PdfReaderWindow::setZoom(qreal factor)
{
    factor = qMax(0.25, qMin(4.0, factor)); // Clamp between 25% and 400%
    m_zoomFactor = factor;
    m_pdfView->setZoomFactor(factor);
    updateZoomControls();
}

// ===== Slot Implementations =====

void PdfReaderWindow::on_backButton_clicked()
{
    emit backRequested();
}

void PdfReaderWindow::on_firstPageButton_clicked()
{
    goToPage(0);
}

void PdfReaderWindow::on_prevPageButton_clicked()
{
    goToPage(m_currentPage - 1);
}

void PdfReaderWindow::on_nextPageButton_clicked()
{
    goToPage(m_currentPage + 1);
}

void PdfReaderWindow::on_lastPageButton_clicked()
{
    goToPage(m_totalPages - 1);
}

void PdfReaderWindow::on_pageEdit_returnPressed()
{
    bool ok;
    int page = ui->pageEdit->text().toInt(&ok) - 1; // Convert to 0-based

    if (ok && page >= 0 && page < m_totalPages) {
        goToPage(page);
    } else {
        ui->pageEdit->setText(QString::number(m_currentPage + 1));
        QMessageBox::information(this, "Invalid Page",
                                 QString("Please enter a valid page number between 1 and %1.").arg(m_totalPages));
    }
}

void PdfReaderWindow::on_pageJumpButton_clicked()
{
    on_pageEdit_returnPressed();
}

void PdfReaderWindow::on_zoomOutButton_clicked()
{
    setZoom(m_zoomFactor - 0.1);
}

void PdfReaderWindow::on_zoomInButton_clicked()
{
    setZoom(m_zoomFactor + 0.1);
}

void PdfReaderWindow::on_fullscreenButton_clicked()
{
    if (isFullScreen()) {
        showNormal();
        ui->fullscreenButton->setText("⛶ FS");
    } else {
        showFullScreen();
        ui->fullscreenButton->setText("⛶ Exit");
    }
}

void PdfReaderWindow::on_zoomCombo_currentIndexChanged(int index)
{
    qreal zoomFactors[] = {0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0};

    if (index >= 0 && index < 8) {
        setZoom(zoomFactors[index]);
    }
}

// ===== Keyboard Event Handling =====

void PdfReaderWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
        on_prevPageButton_clicked();
        break;

    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_Space:
        on_nextPageButton_clicked();
        break;

    case Qt::Key_Home:
        on_firstPageButton_clicked();
        break;

    case Qt::Key_End:
        on_lastPageButton_clicked();
        break;

    case Qt::Key_F:
    case Qt::Key_F11:
        on_fullscreenButton_clicked();
        break;

    case Qt::Key_Escape:
        if (isFullScreen()) {
            on_fullscreenButton_clicked();
        } else {
            on_backButton_clicked();
        }
        break;

    case Qt::Key_Backspace:
        on_backButton_clicked();
        break;

    case Qt::Key_Plus:
    case Qt::Key_Equal:
        on_zoomInButton_clicked();
        break;

    case Qt::Key_Minus:
        on_zoomOutButton_clicked();
        break;

    case Qt::Key_0:
        setZoom(1.0);
        break;

    default:
        QWidget::keyPressEvent(event);
        break;
    }
}
*/
