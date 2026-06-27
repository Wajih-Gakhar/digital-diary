#include "addentrydialog.h"
#include "ui_addentrydialog.h"
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QDateTime>

AddEntryDialog::AddEntryDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::AddEntryDialog)
{
    ui->setupUi(this);

    populateCategories();

    // Set defaults: today's date and current time
    ui->calendar->setSelectedDate(QDate::currentDate());

    QTime currentTime = QTime::currentTime();
    int hour = currentTime.hour();
    int minute = currentTime.minute();

    // Convert to 12-hour format
    if (hour == 0) {
        ui->spinHour->setValue(12);
        ui->cmbAMPM->setCurrentIndex(0); // AM
    }
    else if (hour < 12) {
        ui->spinHour->setValue(hour);
        ui->cmbAMPM->setCurrentIndex(0); // AM
    }
    else if (hour == 12) {
        ui->spinHour->setValue(12);
        ui->cmbAMPM->setCurrentIndex(1); // PM
    }
    else {
        ui->spinHour->setValue(hour - 12);
        ui->cmbAMPM->setCurrentIndex(1); // PM
    }

    ui->spinMinute->setValue(minute);

    // Connect signals
    connect(ui->btnSave, &QPushButton::clicked, this, &AddEntryDialog::on_btnSave_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &AddEntryDialog::on_btnCancel_clicked);
}

AddEntryDialog::~AddEntryDialog()
{
    delete ui;
}

void AddEntryDialog::populateCategories()
{
    ui->cmbCategory->clear();
    ui->cmbCategory->addItem("💖 Personal");
    ui->cmbCategory->addItem("💼 Work");
    ui->cmbCategory->addItem("📚 Study");
    ui->cmbCategory->addItem("💡 Ideas");
    ui->cmbCategory->addItem("⭐ Important");
    ui->cmbCategory->addItem("📌 Other");
}

QString AddEntryDialog::getTitle() const
{
    return ui->txtTitle->text().trimmed();
}

QString AddEntryDialog::getDateTime() const
{
    QDate date = ui->calendar->selectedDate();

    int hour = ui->spinHour->value();
    int minute = ui->spinMinute->value();
    QString ampm = ui->cmbAMPM->currentText();

    // Format: "12 Dec 2025, 02:30 PM"
    QString dateStr = date.toString("dd MMM yyyy");
    QString timeStr = QString("%1:%2 %3")
        .arg(hour, 2, 10, QChar('0'))
        .arg(minute, 2, 10, QChar('0'))
        .arg(ampm);

    return dateStr + ", " + timeStr;
}

QString AddEntryDialog::getCategory() const
{
    QString cat = ui->cmbCategory->currentText();
    // Remove emoji prefix for storage
    if (cat.contains(" ")) {
        return cat.split(" ").last();
    }
    return cat;
}

QString AddEntryDialog::getContent() const
{
    return ui->txtContent->toPlainText().trimmed();
}

void AddEntryDialog::setTitle(const QString& t)
{
    ui->txtTitle->setText(t);
}

void AddEntryDialog::setDateTime(const QString& dt)
{
    // Parse "12 Dec 2025, 02:30 PM" format
    QStringList parts = dt.split(", ");
    if (parts.size() >= 2) {
        QString dateStr = parts[0];
        QString timeStr = parts[1];

        QDate date = QDate::fromString(dateStr, "dd MMM yyyy");
        if (date.isValid()) {
            ui->calendar->setSelectedDate(date);
        }

        // Parse time "02:30 PM"
        QStringList timeParts = timeStr.split(" ");
        if (timeParts.size() >= 2) {
            QString hourMin = timeParts[0];
            QString ampm = timeParts[1];

            QStringList hmParts = hourMin.split(":");
            if (hmParts.size() >= 2) {
                int hour = hmParts[0].toInt();
                int minute = hmParts[1].toInt();

                ui->spinHour->setValue(hour);
                ui->spinMinute->setValue(minute);

                if (ampm == "AM") {
                    ui->cmbAMPM->setCurrentIndex(0);
                }
                else {
                    ui->cmbAMPM->setCurrentIndex(1);
                }
            }
        }
    }
}

void AddEntryDialog::setCategory(const QString& cat)
{
    // Find matching category (with or without emoji)
    for (int i = 0; i < ui->cmbCategory->count(); ++i) {
        QString itemText = ui->cmbCategory->itemText(i);
        if (itemText.contains(cat, Qt::CaseInsensitive)) {
            ui->cmbCategory->setCurrentIndex(i);
            return;
        }
    }
}

void AddEntryDialog::setContent(const QString& c)
{
    ui->txtContent->setPlainText(c);
}

void AddEntryDialog::on_btnSave_clicked()
{
    // Validation
    if (ui->txtTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
            "📝 Title cannot be empty!\n\nPlease enter a title for your diary entry.");
        ui->txtTitle->setFocus();
        return;
    }

    if (getContent().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
            "📄 Content cannot be empty!\n\nPlease write something in your diary entry.");
        ui->txtContent->setFocus();
        return;
    }

    accept();
}

void AddEntryDialog::on_btnCancel_clicked()
{
    reject();
}