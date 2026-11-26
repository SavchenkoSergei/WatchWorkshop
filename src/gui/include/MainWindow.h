#pragma once
#include <QKeySequence>
#include <QList>
#include <QMainWindow>

#include "Account.h"
#include "DialogTypes.h"
#include "Exceptions.h"
#include "OrderStatus.h"

class QAbstractItemModel;
class ClientTableModel;
class WatchTableModel;
class MasterTableModel;
class OrderTableModel;
class PartTableModel;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QStackedWidget;
class QTableView;
class QTimer;
class LogTableModel;
class SortProxy;
class Workshop;

// Главное окно. Вся логика предметной области находится в Workshop,
// обработчики только собирают данные из диалогов и вызывают его методы.
// Набор доступных действий и видимых разделов зависит от роли:
// гость только смотрит; мастер берёт заказы и работает с ними и со складом;
// ресепшен принимает клиентов, часы и детали, оформляет заказы и выдаёт готовые;
// администратор может всё.
class MainWindow : public QMainWindow {
public:
    // masterId - мастер, связанный с учётной записью (0 у остальных ролей)
    MainWindow(Workshop* workshop, Role role, int accountId, const QString& userName, int masterId,
               QWidget* parent = nullptr);
    bool wantsRelogin() const { return relogin; }      // пользователь выбрал «Сменить пользователя»

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // разделы: 0 - клиенты, 1 - часы, 2 - мастера, 3 - заказы, 4 - детали, 5 - журнал (администратор)
    enum Tab { ClientsTab = 0, WatchesTab, MastersTab, OrdersTab, PartsTab, LogTab };
    // кто может выполнить действие (битовая маска ролей)
    enum Who { ForGuest = 1, ForMaster = 2, ForReception = 4, ForAdmin = 8 };
    static constexpr int Everyone = ForGuest | ForMaster | ForReception | ForAdmin;
    static constexpr int Staff = ForMaster | ForReception | ForAdmin;
    static constexpr int FrontDesk = ForReception | ForAdmin;       // ресепшен и администратор
    static constexpr int WorkshopStaff = ForMaster | ForAdmin;       // мастер и администратор
    static constexpr int AdminOnly = ForAdmin;

    void onAddClient();
    void onAddWatch();
    void onAddMaster();
    void onAddPart();
    void onAddCurrent();
    void onEdit();
    void onDelete();
    void onCreateOrder();
    void onTakeOrder();
    void onAssignMaster();
    void onAddService();
    void onRemoveService();
    void onAddUsedPart();
    void onRemoveUsedPart();
    void onChangeStatus();
    void onRestockPart();
    void onWriteOffPart();
    void onCreateAccount();
    void onRemoveAccount();
    void onChangePassword();
    void onSwitchUser();
    void onToggleTheme();
    void onOpenDataFolder();
    void showError(const WorkshopException& e);

    // действия, общие для меню и контекстных меню по правой кнопке мыши
    void addWatchFor(int clientId);
    void createOrderFor(int clientId, int watchId);
    void applyStatus(int orderId, OrderStatus status);
    void applyMaster(int orderId, int masterId);
    void applyUsedPart(int orderId, int partId);
    void showContextMenu(Tab tab, const QPoint& pos);
    void showDetails(Tab tab);                          // полная информация о заказе или часах
    QString orderDetails(int orderId) const;
    QString watchDetails(int watchId) const;

    // оформление окна
    void buildUi();
    void createMenus();
    void createShortcuts();
    void setPage(Tab tab);
    void updateAddButton();
    void updateSummary();                               // карточки со счётчиками заказов
    bool pageVisible(Tab tab) const;
    int addPermission(Tab tab) const;
    QWidget* makeCard(const QString& tone, const QString& caption, QLabel*& valueLabel);
    QTableView* addTable(QAbstractItemModel* model, Tab tab);
    void fitColumns(Tab tab);                           // ширина столбцов по содержимому, пока её не меняли вручную

    bool allowed(int who) const;
    bool ownsOrder(int orderId) const;                  // мастер взял этот заказ
    QMenu* newMenu(const QString& title);
    void addMenuAction(QMenu* menu, const QString& title, void (MainWindow::*slot)(), int who,
                       const QKeySequence& shortcut = QKeySequence());
    void attach(QMenu* menu);
    int selectedId(Tab tab) const;                     // id выбранной строки (столбец 0)
    Tab currentTab() const;
    bool pick(const QString& title, const Choices& choices, int& id);   // выбор из списка
    Choices clientChoices() const;
    Choices watchChoices(bool freeOnly = false) const;      // freeOnly - только часы без незавершённого заказа
    Choices masterChoices() const;
    Choices partChoices() const;
    Choices orderChoices(bool ownOnly = false) const;       // ownOnly - только заказы, взятые этим мастером
    Choices accountChoices() const;
    void refreshTab(Tab tab);
    void refreshAll();

    template <typename F>
    void guarded(F&& action);                          // выполняет действие, ошибки - в окно

    Workshop* workshop;
    Role role;
    int accountId;
    QString userName;
    int masterId;
    bool relogin = false;
    bool darkTheme = false;
    bool autoResizing = false;
    bool userSized[6] = {false, false, false, false, false, false};
    QString lastDate;                                  // чтобы обновлять сроки при смене даты
    QStackedWidget* pages;
    QLabel* titleLabel;
    QLineEdit* searchEdit;
    QPushButton* addButton;
    QPushButton* takeButton;
    QWidget* cardsBar;
    QLabel* cardValues[4];
    QPushButton* navButtons[6];
    QTimer* timer;
    ClientTableModel* clientModel;
    WatchTableModel* watchModel;
    OrderTableModel* orderModel;
    PartTableModel* partModel;
    MasterTableModel* masterModel;
    LogTableModel* logModel;
    SortProxy* proxies[6];
    QTableView* tables[6];
};
