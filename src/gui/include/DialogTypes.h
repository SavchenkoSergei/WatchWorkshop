#pragma once
#include <QList>
#include <QString>

// Элемент списка выбора в диалогах (клиент, часы, мастер).
// parentId у часов - идентификатор владельца; mechanism и brand нужны только для часов.
struct Choice {
    int id = 0;
    QString title;
    int parentId = 0;
    QString mechanism;
    QString brand;
};
using Choices = QList<Choice>;
