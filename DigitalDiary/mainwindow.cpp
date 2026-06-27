#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addentrydialog.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QCalendarWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Table setup
    ui->tableDiary->setColumnCount(4);
    ui->tableDiary->setHorizontalHeaderLabels({ "📝 Title", "📅 Date & Time", "🏷️ Category", "📄 Content" });
    ui->tableDiary->horizontalHeader()->setStretchLastSection(true);
    ui->tableDiary->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableDiary->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableDiary->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableDiary->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableDiary->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::addEntry);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindow::editEntry);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::deleteEntry);
    connect(ui->btnSearch, &QPushButton::clicked, this, &MainWindow::searchEntries);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveToFileSlot);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::loadFromFileSlot);
    connect(ui->btnMenu, &QPushButton::clicked, this, &MainWindow::toggleSidebar);
    connect(ui->btnFAB, &QPushButton::clicked, this, &MainWindow::addEntry);

    connect(ui->btnAddSide, &QPushButton::clicked, this, [this]() {
        toggleSidebar();  // Close sidebar first
        addEntry();       // Then open add dialog
        });

    connect(ui->btnHome, &QPushButton::clicked, this, &MainWindow::toggleSidebar);

    connect(ui->btnCalendarSide, &QPushButton::clicked, this, [this]() {
        toggleSidebar();        // Close sidebar
        showCalendarView();     // Open calendar dialog
        });

    connect(ui->btnCategoriesSide, &QPushButton::clicked, this, [this]() {
        toggleSidebar();          // Close sidebar
        showCategoriesView();     // Open categories dialog
        });

    connect(ui->txtSearch, &QLineEdit::returnPressed, this, &MainWindow::searchEntries);

    // Initial UI setup
    ui->sidebar->move(-250, 0);
    ui->sidebar->raise();
    ui->btnFAB->raise();

    // Load saved diary
    loadFromFile("diary.txt");
    refreshTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    int sidebarX = sidebarOpen ? 0 : -250;
    ui->sidebar->setGeometry(sidebarX, 0, 250, height());

    int margin = 30;
    int fw = ui->btnFAB->width();
    int fh = ui->btnFAB->height();
    ui->btnFAB->move(width() - fw - margin, height() - fh - margin);
}

void MainWindow::refreshTable()
{
    ui->tableDiary->setRowCount((int)entries.size());
    for (int i = 0; i < (int)entries.size(); ++i) {
        const DiaryEntry& e = entries[i];
        auto* it0 = new QTableWidgetItem(e.title);
        auto* it1 = new QTableWidgetItem(e.dateTime);
        auto* it2 = new QTableWidgetItem(e.category);
        auto* it3 = new QTableWidgetItem(e.content);

        QFont titleFont;
        titleFont.setBold(true);
        titleFont.setPointSize(10);
        it0->setFont(titleFont);

        QColor c = categoryColor(e.category);
        it2->setBackground(c);
        it2->setForeground((c.lightness() < 180) ? Qt::white : Qt::black);

        QFont catFont;
        catFont.setBold(true);
        it2->setFont(catFont);
        it2->setTextAlignment(Qt::AlignCenter);

        ui->tableDiary->setItem(i, 0, it0);
        ui->tableDiary->setItem(i, 1, it1);
        ui->tableDiary->setItem(i, 2, it2);
        ui->tableDiary->setItem(i, 3, it3);
    }
    ui->tableDiary->resizeRowsToContents();
}

QColor MainWindow::categoryColor(const QString& cat) const
{
    if (cat == "Personal") return QColor("#FFB6C1");
    if (cat == "Work")     return QColor("#87CEEB");
    if (cat == "Study")    return QColor("#FFE4B5");
    if (cat == "Ideas")    return QColor("#DDA0DD");
    if (cat == "Important")return QColor("#FFB6B6");
    return QColor("#E0E0E0");
}

void MainWindow::addEntry()
{
    AddEntryDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        DiaryEntry e(dlg.getTitle(), dlg.getDateTime(), dlg.getCategory(), dlg.getContent());
        entries.push_back(e);
        refreshTable();
    }
}

void MainWindow::editEntry()
{
    int row = ui->tableDiary->currentRow();
    if (row < 0 || row >= (int)entries.size()) {
        QMessageBox::warning(this, "Edit", "Please select a row to edit.");
        return;
    }

    AddEntryDialog dlg(this);
    dlg.setWindowTitle("✏️ Edit Diary Entry");
    dlg.setTitle(entries[row].title);
    dlg.setDateTime(entries[row].dateTime);
    dlg.setCategory(entries[row].category);
    dlg.setContent(entries[row].content);

    if (dlg.exec() == QDialog::Accepted) {
        entries[row].title = dlg.getTitle();
        entries[row].dateTime = dlg.getDateTime();
        entries[row].category = dlg.getCategory();
        entries[row].content = dlg.getContent();
        refreshTable();
    }
}

void MainWindow::deleteEntry()
{
    int row = ui->tableDiary->currentRow();
    if (row < 0 || row >= (int)entries.size()) {
        QMessageBox::information(this, "Delete", "Please select an entry first.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        "Are you sure you want to delete this entry?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        entries.erase(entries.begin() + row);
        refreshTable();
        QMessageBox::information(this, "Deleted", "Entry deleted successfully! 🗑️");
    }
}

void MainWindow::searchEntries()
{
    QString keyword = ui->txtSearch->text().trimmed();

    // Check if search field is empty
    if (keyword.isEmpty()) {
        QMessageBox::information(this, "Search",
            "Please enter a search keyword.\n\n💡 Try searching for a title, date, or category!");
        return;
    }

    int startRow = ui->tableDiary->currentRow() + 1;
    if (startRow >= (int)entries.size()) {
        startRow = 0;
    }

    // Search forward from start position to end
    for (int i = startRow; i < (int)entries.size(); ++i) {
        if (matchesSearch(entries[i], keyword)) {
            highlightResult(i);
            return;
        }
    }

    for (int i = 0; i < startRow; ++i) {
        if (matchesSearch(entries[i], keyword)) {
            highlightResult(i);
            return;
        }
    }

    // No match found anywhere
    QMessageBox::information(this, "Not Found",
        QString("No matching entry found for \"%1\".\n\n💡 Try different keywords!").arg(keyword));
}

bool MainWindow::matchesSearch(const DiaryEntry& entry, const QString& keyword) const
{
    return entry.title.contains(keyword, Qt::CaseInsensitive) ||
        entry.content.contains(keyword, Qt::CaseInsensitive) ||
        entry.dateTime.contains(keyword, Qt::CaseInsensitive) ||
        entry.category.contains(keyword, Qt::CaseInsensitive);
}

void MainWindow::highlightResult(int row)
{
    // Select and scroll to the row
    ui->tableDiary->selectRow(row);
    ui->tableDiary->scrollToItem(ui->tableDiary->item(row, 0), QAbstractItemView::PositionAtCenter);

    // Show detailed information about found entry
    const DiaryEntry& e = entries[row];
    QMessageBox::information(this, "Found 🔍",
        QString("Entry found!\n\n"
            "📝 Title: %1\n"
            "📅 Date: %2\n"
            "🏷️ Category: %3\n\n"
            "💡 Press Search again to find next occurrence!")
        .arg(e.title, e.dateTime, e.category));
}

void MainWindow::saveToFileSlot()
{
    saveToFile("diary.txt");
    QMessageBox::information(this, "Saved 💾",
        QString("Your diary has been saved successfully!\n\n"
            "📊 Total entries: %1").arg(entries.size()));
}

void MainWindow::loadFromFileSlot()
{
    loadFromFile("diary.txt");
    refreshTable();
    QMessageBox::information(this, "Loaded 📂",
        QString("Diary loaded successfully!\n\n"
            "📊 %1 entries found.").arg(entries.size()));
}

void MainWindow::showCalendarView()
{
    // Create calendar dialog
    QDialog* calDlg = new QDialog(this);
    calDlg->setWindowTitle("📅 Calendar View");
    calDlg->setMinimumSize(550, 550);

    QVBoxLayout* layout = new QVBoxLayout(calDlg);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    // Title label at top
    QLabel* titleLabel = new QLabel("📅 Your Diary Entries by Date", calDlg);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #4A148C; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Calendar widget
    QCalendarWidget* calendar = new QCalendarWidget(calDlg);
    calendar->setMinimumHeight(300);
    calendar->setMaximumHeight(350);

    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(QColor("#FFB6D9"));
    highlightFormat.setForeground(Qt::white);
    highlightFormat.setFontWeight(QFont::Bold);

    // Loop through all entries and mark their dates on calendar
    for (const auto& entry : entries) {
        // Parse date from "12 Dec 2025, 02:30 PM" format
        QString dateStr = entry.dateTime.split(",").first().trimmed();
        QDate date = QDate::fromString(dateStr, "dd MMM yyyy");
        if (date.isValid()) {
            calendar->setDateTextFormat(date, highlightFormat);
        }
    }

    layout->addWidget(calendar);

    QLabel* infoLabel = new QLabel("💡 Pink highlighted dates have diary entries! Click to view.", calDlg);
    infoLabel->setStyleSheet(
        "color: #666; "
        "padding: 10px; "
        "font-style: italic; "
        "background: #FFF5FA; "
        "border: 1px solid #FFE4F0; "
        "border-radius: 8px;"
    );
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // Entries section with label
    QLabel* entriesLabel = new QLabel("📝 Entries for selected date:", calDlg);
    entriesLabel->setStyleSheet("font-weight: bold; padding-top: 5px; color: #4A148C;");
    layout->addWidget(entriesLabel);

    // List widget for entries
    QListWidget* entriesList = new QListWidget(calDlg);
    entriesList->setMinimumHeight(100);
    entriesList->setMaximumHeight(120);
    entriesList->setStyleSheet(
        "QListWidget { "
        "  background: white; "
        "  border: 2px solid #FFB6D9; "
        "  border-radius: 8px; "
        "  padding: 5px; "
        "}"
        "QListWidget::item { "
        "  padding: 5px; "
        "  border-bottom: 1px solid #FFE4F0; "
        "}"
    );
    layout->addWidget(entriesList);

    connect(calendar, &QCalendarWidget::selectionChanged, [this, calendar, entriesList]() {
        entriesList->clear();
        QDate selected = calendar->selectedDate();
        QString selectedStr = selected.toString("dd MMM yyyy");

        int count = 0;
        for (const auto& entry : entries) {
            QString entryDateStr = entry.dateTime.split(",").first().trimmed();
            if (entryDateStr == selectedStr) {
                QString timeStr = entry.dateTime.split(",").last().trimmed();
                entriesList->addItem(QString("⏰ %1 - 📝 %2 (%3)")
                    .arg(timeStr, entry.title, entry.category));
                count++;
            }
        }

        if (count == 0) {
            QListWidgetItem* emptyItem = new QListWidgetItem("📭 No entries for this date");
            emptyItem->setForeground(QColor("#999"));
            emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
            entriesList->addItem(emptyItem);
        }
        });

    // Trigger initial selection to show today's entries
    QDate today = calendar->selectedDate();
    QString todayStr = today.toString("dd MMM yyyy");
    int todayCount = 0;
    for (const auto& entry : entries) {
        QString entryDateStr = entry.dateTime.split(",").first().trimmed();
        if (entryDateStr == todayStr) {
            QString timeStr = entry.dateTime.split(",").last().trimmed();
            entriesList->addItem(QString("⏰ %1 - 📝 %2 (%3)")
                .arg(timeStr, entry.title, entry.category));
            todayCount++;
        }
    }
    if (todayCount == 0) {
        QListWidgetItem* emptyItem = new QListWidgetItem("📭 No entries for this date");
        emptyItem->setForeground(QColor("#999"));
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
        entriesList->addItem(emptyItem);
    }

    // Close button at bottom
    QPushButton* closeBtn = new QPushButton("✅ Close", calDlg);
    closeBtn->setMinimumHeight(45);
    closeBtn->setStyleSheet(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFB6D9, stop:1 #D16BA5); "
        "  color: white; "
        "  font-weight: bold; "
        "  font-size: 14px; "
        "  border-radius: 10px; "
        "  padding: 10px; "
        "}"
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFA0C9, stop:1 #C15294); "
        "}"
    );
    connect(closeBtn, &QPushButton::clicked, calDlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    calDlg->exec();
    delete calDlg;
}

void MainWindow::showCategoriesView()
{
    // Create categories dialog
    QDialog* catDlg = new QDialog(this);
    catDlg->setWindowTitle("📚 Categories Summary");
    catDlg->setMinimumSize(500, 450);

    QVBoxLayout* layout = new QVBoxLayout(catDlg);

    // Title label
    QLabel* titleLabel = new QLabel("📚 Entries by Category", catDlg);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #4A148C; padding: 10px;");
    layout->addWidget(titleLabel);

    // FIX: Count entries by category
    QMap<QString, int> categoryCounts;
    QMap<QString, QStringList> categoryEntries;

    for (const auto& entry : entries) {
        categoryCounts[entry.category]++;
        categoryEntries[entry.category].append(
            QString("%1 (%2)").arg(entry.title, entry.dateTime.split(",").first())
        );
    }

    // Display category summary with counts
    QLabel* summaryLabel = new QLabel("💡 Click a category to see its entries:", catDlg);
    summaryLabel->setStyleSheet("color: #666; padding: 5px; font-style: italic;");
    layout->addWidget(summaryLabel);

    QListWidget* categoryList = new QListWidget(catDlg);
    categoryList->setMinimumHeight(180);

    // FIX: Create list items for each category with emoji and color
    QStringList categories = { "Personal", "Work", "Study", "Ideas", "Important", "Other" };
    for (const QString& cat : categories) {
        int count = categoryCounts.value(cat, 0);
        if (count > 0) {
            QString emoji;
            if (cat == "Personal") emoji = "💖";
            else if (cat == "Work") emoji = "💼";
            else if (cat == "Study") emoji = "📚";
            else if (cat == "Ideas") emoji = "💡";
            else if (cat == "Important") emoji = "⭐";
            else emoji = "📌";

            QListWidgetItem* item = new QListWidgetItem(
                QString("%1 %2: %3 %4").arg(emoji, cat).arg(count).arg(count == 1 ? "entry" : "entries"),
                categoryList
            );

            // FIX: Apply category color to list item
            item->setBackground(categoryColor(cat));
            item->setForeground(Qt::black);

            QFont font = item->font();
            font.setBold(true);
            font.setPointSize(11);
            item->setFont(font);
        }
    }

    if (categoryList->count() == 0) {
        QListWidgetItem* emptyItem = new QListWidgetItem("📭 No entries yet. Start writing! ✨", categoryList);
        emptyItem->setForeground(QColor("#999"));
    }

    layout->addWidget(categoryList);

    // Entries list for selected category
    QLabel* entriesLabel = new QLabel("📝 Entries in selected category:", catDlg);
    entriesLabel->setStyleSheet("font-weight: bold; padding-top: 10px;");
    layout->addWidget(entriesLabel);

    QListWidget* entriesList = new QListWidget(catDlg);
    entriesList->setMaximumHeight(150);
    layout->addWidget(entriesList);

    // FIX: Show entries when category is clicked
    connect(categoryList, &QListWidget::itemClicked, [categoryEntries, entriesList](QListWidgetItem* item) {
        entriesList->clear();
        QString text = item->text();

        // Extract category name from "💖 Personal: 5 entries" format
        QString category = text.split(":").first().split(" ").last();

        if (categoryEntries.contains(category)) {
            for (const QString& entryInfo : categoryEntries[category]) {
                entriesList->addItem("• " + entryInfo);
            }
        }
        });

    // Close button
    QPushButton* closeBtn = new QPushButton("✅ Close", catDlg);
    closeBtn->setMinimumHeight(40);
    connect(closeBtn, &QPushButton::clicked, catDlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    catDlg->exec();
    delete catDlg;
}

void MainWindow::toggleSidebar()
{
    if (isAnimating) return;

    isAnimating = true;
    int sw = 250;
    QRect start, end;

    if (!sidebarOpen) {
        start = ui->sidebar->geometry();
        end = QRect(0, 0, sw, height());
    }
    else {
        start = ui->sidebar->geometry();
        end = QRect(-sw, 0, sw, height());
    }

    QPropertyAnimation* anim = new QPropertyAnimation(ui->sidebar, "geometry");
    anim->setDuration(400);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(start);
    anim->setEndValue(end);

    connect(anim, &QPropertyAnimation::finished, [this]() {
        isAnimating = false;
        });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
    sidebarOpen = !sidebarOpen;
}

void MainWindow::loadFromFile(const QString& filename)
{
    QFile f(filename);
    if (!f.exists()) return;
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    entries.clear();
    QTextStream in(&f);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif

    while (!in.atEnd()) {
        QString title = in.readLine();
        if (title.isEmpty()) continue;

        QString dateTime = in.readLine();
        QString category = in.readLine();
        QString content;
        QString line;

        while (!in.atEnd()) {
            line = in.readLine();
            if (line == "---END---") break;
            if (!content.isEmpty()) content += "\n";
            content += line;
        }

        entries.emplace_back(title, dateTime, category, content);
    }
    f.close();
}

void MainWindow::saveToFile(const QString& filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&f);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif

    for (const auto& e : entries) {
        out << e.title << "\n";
        out << e.dateTime << "\n";
        out << e.category << "\n";
        out << e.content << "\n";
        out << "---END---\n";
    }
    f.close();
}