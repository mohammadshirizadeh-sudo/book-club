#ifndef BOOKREVIEWDIALOG_H
#define BOOKREVIEWDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"

namespace Ui {
class BookReviewDialog;
}

class BookReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookReviewDialog(NetworkManager* networkManager,
                              const QVariantMap& bookData,
                              QWidget *parent = nullptr);
    ~BookReviewDialog();

private slots:
    void handleResponse(const Response& response);

    void on_closeButton_clicked();
    void on_star1Button_clicked();
    void on_star2Button_clicked();
    void on_star3Button_clicked();
    void on_star4Button_clicked();
    void on_star5Button_clicked();
    void on_reviewTextEdit_textChanged();
    void on_submitReviewButton_clicked();

private:
    Ui::BookReviewDialog *ui;
    NetworkManager* m_networkManager;
    QVariantMap m_bookData;
    int m_bookId = 0;

    int m_selectedRating = 0;

    bool m_hasExistingReview = false;
    int  m_existingReviewId  = -1;
    int  m_existingRating    = 0;

    static const int MAX_REVIEW_LENGTH = 1000;

    void setupBookInfo();
    void requestReviews();
    void requestAddOrEditReview();
    void setStarRating(int rating);
    void updateSubmitButtonState();
    void populateReviews(const QVariantList& reviews);
    void clearReviewsContainer();
    QString formatTimeAgo(const QDateTime& dt) const;
};

#endif // BOOKREVIEWDIALOG_H