#ifndef DIARYENTRY_H
#define DIARYENTRY_H

#include <QString>

struct DiaryEntry
{
    QString title;
    QString dateTime;  // formatted date+time string (e.g. "12 Dec 2025, 02:30 PM")
    QString category;
    QString content;

    DiaryEntry() {}

    DiaryEntry(const QString& t, const QString& dt, const QString& c, const QString& cont)
        : title(t), dateTime(dt), category(c), content(cont)
    {
    }
};

#endif // DIARYENTRY_H