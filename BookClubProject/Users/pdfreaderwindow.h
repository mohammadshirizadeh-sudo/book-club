

/*
#ifndef PDFREADERWINDOW_H
#define PDFREADERWINDOW_H

#include <QWidget>

// #include <QPdfDocument>
// #include <QPdfView>

#include <QPdfDocument>
#include <QtPdfWidgets/QPdfView>


namespace Ui {
class PdfReaderWindow;
}

class PdfReaderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PdfReaderWindow(int bookId, QWidget *parent = nullptr);
    ~PdfReaderWindow();

    bool loadPdf(const QString &filePath);
    void setBookTitle(const QString &title);

signals:
    void backRequested();
    void pageChanged(int currentPage);

private slots:
    void on_backButton_clicked();
    void on_firstPageButton_clicked();
    void on_prevPageButton_clicked();
    void on_nextPageButton_clicked();
    void on_lastPageButton_clicked();
    void on_pageEdit_returnPressed();
    void on_pageJumpButton_clicked();
    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void on_fullscreenButton_clicked();
    void on_zoomCombo_currentIndexChanged(int index);

    void on_zoomStatusLabel_linkActivated(const QString &link);

private:
    Ui::PdfReaderWindow *ui;

    // QPdfDocument *m_pdfDocument;
    // QPdfView *m_pdfView;

    int m_bookId;
    int m_currentPage;
    int m_totalPages;
    qreal m_zoomFactor;

    void setupPdfView();
    void updatePageInfo();
    void updateZoomControls();
    void goToPage(int page);
    void setZoom(qreal factor);
};

#endif // PDFREADERWINDOW_H


*/
