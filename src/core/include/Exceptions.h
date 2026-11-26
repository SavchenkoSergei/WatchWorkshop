#pragma once

#include <stdexcept>
#include <string>

class WorkshopException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class InvalidInputException : public WorkshopException {
public:
    explicit InvalidInputException(const std::string& fieldName)
        : WorkshopException("Некорректное значение: " + fieldName), fieldName(fieldName) {}
    const std::string& getFieldName() const { return fieldName; }
private:
    std::string fieldName;
};

class ObjectNotFoundException : public WorkshopException {
public:
    explicit ObjectNotFoundException(int objectId)
        : WorkshopException("Объект не найден, id = " + std::to_string(objectId)),
          objectId(objectId) {}
    int getObjectId() const { return objectId; }
private:
    int objectId;
};

class AccessDeniedException : public WorkshopException {
public:
    explicit AccessDeniedException(const std::string& action)
        : WorkshopException("Недостаточно прав: " + action), action(action) {}
    const std::string& getAction() const { return action; }
private:
    std::string action;
};

class ObjectInUseException : public WorkshopException {
public:
    explicit ObjectInUseException(const std::string& reason)
        : WorkshopException("Нельзя удалить: " + reason), reason(reason) {}
    const std::string& getReason() const { return reason; }
private:
    std::string reason;
};

class OutOfStockException : public WorkshopException {
public:
    explicit OutOfStockException(const std::string& partName)
        : WorkshopException("Недостаточно на складе: " + partName), partName(partName) {}
    const std::string& getPartName() const { return partName; }
private:
    std::string partName;
};

class FileCorruptedException : public WorkshopException {
public:
    explicit FileCorruptedException(const std::string& fileName)
        : WorkshopException("Файл повреждён или недоступен: " + fileName), fileName(fileName) {}
    const std::string& getFileName() const { return fileName; }
private:
    std::string fileName;
};
