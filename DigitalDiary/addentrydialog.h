#ifndef ADDENTRYDIALOG_H
#define ADDENTRYDIALOG_H

#include <QDialog>

namespace Ui { class AddEntryDialog; }

class AddEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEntryDialog(QWidget* parent = nullptr);
    ~AddEntryDialog();

    // Getters
    QString getTitle() const;
    QString getDateTime() const; // Returns formatted date+time string
    QString getCategory() const;
    QString getContent() const;

    // Setters (for edit mode)
    void setTitle(const QString& t);
    void setDateTime(const QString& dt); // Accepts formatted string like "12 Dec 2025, 02:30 PM"
    void setCategory(const QString& cat);
    void setContent(const QString& c);

private slots:
    void on_btnSave_clicked();
    void on_btnCancel_clicked();

private:
    Ui::AddEntryDialog* ui;
    void populateCategories();
};

#endif // ADDENTRYDIALOG_H