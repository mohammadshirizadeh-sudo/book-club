// genrewindow.cpp
#include "genrewindow.h"
#include "appWindow/ui_genrewindow.h"
#include <QMessageBox>

GenreWindow::GenreWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GenreWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    genreCheckBoxes = {
        ui->Adventure,
        ui->Art,
        ui->Biography,
        ui->Business,
        ui->Comedy,
        ui->Comics,
        ui->Cooking,
        ui->Documentation,
        ui->Drama,
        ui->Education,
        ui->Fantacy,
        ui->Fiction,
        ui->Health,
        ui->History,
        ui->Horror,
        ui->LanguageLearning,
        ui->Lgbtq,
        ui->Manga,
        ui->Music,
        ui->MysteryCrime,
        ui->Other,
        ui->PersonalDevelopment,
        ui->Philosophy,
        ui->Poetry,
        ui->PoliticsSociety,
        ui->Psycology,
        ui->Refrence,
        ui->Romance,
        ui->Science,
        ui->ScienceFiction,
        ui->Technology,
        ui->Thriller
    };

    for (QCheckBox *cb : genreCheckBoxes) {
        connect(cb, &QCheckBox::checkStateChanged,
                this, &GenreWindow::updateGenreCheckBoxes);
    }

    // اتصال به پاسخ سرور
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &GenreWindow::handleGenreResponse);
}

GenreWindow::~GenreWindow()
{
    delete ui;
}

void GenreWindow::updateGenreCheckBoxes()
{
    int checkedCount = 0;
    for (QCheckBox *cb : genreCheckBoxes) {
        if (cb->isChecked()) checkedCount++;
    }

    for (QCheckBox *cb : genreCheckBoxes) {
        if (!cb->isChecked()) {
            cb->setEnabled(checkedCount < 3);
        }
    }
}

QVector<Genre> GenreWindow::getSelectedGenres() const
{
    QVector<Genre> selected;
    for (QCheckBox *cb : genreCheckBoxes) {
        if (cb->isChecked()) {
            // تبدیل متن چک‌باکس به Genre
            QString text = cb->text();
            Genre genre = GenreHelper::fromString(text);
            if (genre != Genre::other) {
                selected.append(genre);
            }
        }
    }
    return selected;
}

void GenreWindow::on_userEnterPushButton_clicked()
{
    QVector<Genre> selectedGenres = getSelectedGenres();
    if (selectedGenres.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select at least one genre.");
        return;
    }

    if (selectedGenres.size() > 3) {
        QMessageBox::warning(this, "Warning", "You can select up to 3 genres.");
        return;
    }
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) {
        QMessageBox::critical(this, "Error", "User not logged in!");
        return;
    }
    QStringList genreStrings;
    for (Genre genre : selectedGenres) {
        genreStrings.append(GenreHelper::toString(genre));
    }
    QVariantMap params;
    params["userId"] = userId;
    params["genres"] = genreStrings;

    Request request(CommandType::UpdateFavoriteGenres, params);
    m_networkManager->sendRequest(request);
    ui->userEnterPushButton->setEnabled(false);
    ui->userEnterPushButton->setText("Saving...");
}

void GenreWindow::handleGenreResponse(const Response& response)
{
    if (response.getCommandType() != CommandType::UpdateFavoriteGenres) {
        return;
    }
    ui->userEnterPushButton->setEnabled(true);
    ui->userEnterPushButton->setText("Enter");

    if (response.isSuccess()) {
        QMessageBox::information(this, "Success", "Genres saved successfully!");
        emit openUserWindow();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save genres: " + response.getMessage());
    }
}