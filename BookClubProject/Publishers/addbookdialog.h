#ifndef ADDBOOKDIALOG_H
#define ADDBOOKDIALOG_H

#include <QDialog>
#include "../NetworkManger/NetworkManager.h"

namespace Ui {
class AddBookDialog;
}

class AddBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBookDialog(NetworkManager* networkManager, int publisherId, QWidget *parent = nullptr);
    ~AddBookDialog();

    void clearForm();

signals:
    void bookAddedSuccessfully();

private slots:
    void on_browseCoverButton_clicked();
    void on_browsePdfButton_clicked();
    void on_clearButton_clicked();
    void on_submitButton_clicked();
    void validateForm();
    void updateCharCount();
    void onResponseReceived(const Response& response);

private:
    Ui::AddBookDialog *ui;
    NetworkManager* m_networkManager;
    int m_publisherId;

    QByteArray m_coverImageData;
    QByteArray m_pdfData;
    QString m_coverFileName;
    QString m_pdfFileName;

    bool isFormValid() const;
    bool validateCoverImage(const QString &filePath);
    bool validatePdfFile(const QString &filePath);
    void updateCoverPreview(const QPixmap &pixmap);
    void updatePdfDisplay(const QString &fileName);
};

#endif // ADDBOOKDIALOG_H
