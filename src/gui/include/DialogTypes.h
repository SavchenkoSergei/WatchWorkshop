#pragma once
#include <QList>
#include <QString>

struct Choice {
    int id = 0;
    QString title;
    int parentId = 0;
    QString mechanism;
    QString brand;
};
using Choices = QList<Choice>;
