#include "genrewindow.h"
#include "ui_genrewindow.h"

#include "userwindow.h"

GenreWindow::GenreWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GenreWindow)
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

    for (QCheckBox *cb : genreCheckBoxes)
    {
        connect(cb, QCheckBox::stateChanged,
                this, &GenreWindow::updateGenreCheckBoxes);
    }
}

GenreWindow::~GenreWindow()
{
    delete ui;
}

void GenreWindow::updateGenreCheckBoxes()
{
    int checkedCount = 0;

    for (QCheckBox *cb : genreCheckBoxes)
    {
        if (cb->isChecked())
            checkedCount++;
    }

    for (QCheckBox *cb : genreCheckBoxes)
    {
        if (!cb->isChecked())
            cb->setEnabled(checkedCount < 3);
    }
}

void GenreWindow::on_userEnterPushButton_clicked()
{
    emit openUserWindow();
}
