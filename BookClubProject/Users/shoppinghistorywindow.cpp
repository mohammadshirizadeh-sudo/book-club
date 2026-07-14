#include "shoppinghistorywindow.h"
#include "ui_shoppinghistorywindow.h"

#include <QHeaderView>
#include <QStandardItem>

ShoppingHistoryWindow::ShoppingHistoryWindow(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ShoppingHistoryWindow),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // ---------- تنظیم Model ----------
    model->setColumnCount(4);

    model->setHorizontalHeaderLabels({
        "Column",
        "Book Name",
        "Author",
        "Amount"
    });

    // ---------- اتصال Model به TableView ----------
    ui->tableView->setModel(model);

    // ---------- تنظیم ظاهر جدول ----------
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->verticalHeader()->setVisible(false);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tableView->setAlternatingRowColors(true);

    ui->tableView->setSortingEnabled(true);

    ui->tableView->setShowGrid(true);

    ui->tableView->setCornerButtonEnabled(false);
}

ShoppingHistoryWindow::~ShoppingHistoryWindow()
{
    delete ui;
}

void ShoppingHistoryWindow::clearHistory()
{
    model->removeRows(0, model->rowCount());
}

void ShoppingHistoryWindow::addPurchase(const QString &bookName,
                                        const QString &author,
                                        const QString &amount)
{
    QList<QStandardItem*> row;

    row << new QStandardItem(QString::number(model->rowCount() + 1));

    row << new QStandardItem(bookName);

    row << new QStandardItem(author);

    row << new QStandardItem(amount);

    model->appendRow(row);
}