#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "diaryentry.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPropertyAnimation;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:

    void addEntry();          
    void editEntry();          
    void deleteEntry();        
    void searchEntries();     
    void saveToFileSlot();     
    void loadFromFileSlot();   
    void toggleSidebar();   

    void showCalendarView();   
    void showCategoriesView(); 

private:
    Ui::MainWindow* ui;
    std::vector<DiaryEntry> entries;
    bool sidebarOpen = false;
    bool isAnimating = false;

    void refreshTable();
    void loadFromFile(const QString& filename);
    void saveToFile(const QString& filename);
    QColor categoryColor(const QString& cat) const;

    bool matchesSearch(const DiaryEntry& entry, const QString& keyword) const;
    void highlightResult(int row);
};

#endif // MAINWINDOW_H