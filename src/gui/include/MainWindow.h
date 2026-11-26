#pragma once
#include <QKeySequence>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <functional>

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
class Order;

class MainWindow : public QMainWindow {
public:
    MainWindow(Workshop* workshop, Role role, int accountId, const QString& userName, int masterId,
               QWidget* parent = nullptr);
    bool wantsRelogin() const { return relogin; }     

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    struct ContextMenuHelper {
        MainWindow* win;
        QMenu* menu;

        void add(const QString& title, int who, std::function<void()> action) {
            if (!win->allowed(who)) return;
            QAction* a = menu->addAction(title);
            QObject::connect(a, &QAction::triggered, win, [action] { action(); });
        }

        void separator() {
            if (!menu->isEmpty() && !menu->actions().last()->isSeparator()) {
                menu->addSeparator();
            }
        }

        void addChoiceSubmenu(const QString& title, const QString& emptyText,
            const Choices& choices, std::function<void(int)> onSelect) {
            QMenu* sub = menu->addMenu(title);
            if (choices.isEmpty()) {
                sub->addAction(emptyText)->setEnabled(false);
                return;
            }
            for (const Choice& c : choices) {
                QAction* action = sub->addAction(c.title);
                int choiceId = c.id;
                QObject::connect(action, &QAction::triggered, win, [onSelect, choiceId] {
                    onSelect(choiceId);
                    });
            }
        }
    };

    void buildClientsMenu(ContextMenuHelper& m, bool hasRow, int id);
    void buildWatchesMenu(ContextMenuHelper& m, bool hasRow, int id);
    void buildMastersMenu(ContextMenuHelper& m, bool hasRow, int id);
    void buildPartsMenu(ContextMenuHelper& m, bool hasRow, int id);
    void buildOrdersMenu(ContextMenuHelper& m, bool hasRow, int id, int column);

    struct OrderMenuContext {
        ContextMenuHelper& m;
        const Order* order;
        int id;
        bool hasRow;
        bool owns;
        bool statusDone = false;
        bool masterDone = false;
        bool serviceDone = false;
        bool partDone = false;
    };

    void addOrderTakeAction(OrderMenuContext& ctx);
    void addOrderStatusMenu(OrderMenuContext& ctx);
    void addOrderMasterMenu(OrderMenuContext& ctx);
    void addOrderServiceMenu(OrderMenuContext& ctx);
    void addOrderPartMenu(OrderMenuContext& ctx);

    enum Tab { ClientsTab = 0, WatchesTab, MastersTab, OrdersTab, PartsTab, LogTab };

    enum Who { ForGuest = 1, ForMaster = 2, ForReception = 4, ForAdmin = 8 };
    static constexpr int Everyone = ForGuest | ForMaster | ForReception | ForAdmin;
    static constexpr int Staff = ForMaster | ForReception | ForAdmin;
    static constexpr int FrontDesk = ForReception | ForAdmin;    
    static constexpr int WorkshopStaff = ForMaster | ForAdmin;      
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


    void addWatchFor(int clientId);
    void createOrderFor(int clientId, int watchId);
    void applyStatus(int orderId, OrderStatus status);
    void applyMaster(int orderId, int masterId);
    void applyUsedPart(int orderId, int partId);
    void showContextMenu(Tab tab, const QPoint& pos);
    void showDetails(Tab tab);                          
    QString orderDetails(int orderId) const;
    QString watchDetails(int watchId) const;


    void buildUi();
    void createMenus();
    void createShortcuts();
    void setPage(Tab tab);
    void updateAddButton();
    void updateSummary();                              
    bool pageVisible(Tab tab) const;
    int addPermission(Tab tab) const;
    QWidget* makeCard(const QString& tone, const QString& caption, QLabel*& valueLabel);
    QTableView* addTable(QAbstractItemModel* model, Tab tab);
    void fitColumns(Tab tab);                          

    bool allowed(int who) const;
    bool ownsOrder(int orderId) const;                  
    QMenu* newMenu(const QString& title);
    void addMenuAction(QMenu* menu, const QString& title, void (MainWindow::*slot)(), int who,
                       const QKeySequence& shortcut = QKeySequence());
    void attach(QMenu* menu);
    int selectedId(Tab tab) const;                
    Tab currentTab() const;
    bool pick(const QString& title, const Choices& choices, int& id);  
    Choices clientChoices() const;
    Choices watchChoices(bool freeOnly = false) const;     
    Choices masterChoices() const;
    Choices partChoices() const;
    Choices orderChoices(bool ownOnly = false) const;       
    Choices accountChoices() const;
    void refreshTab(Tab tab);
    void refreshAll();

    template <typename F>
    void guarded(F&& action);                        

    Workshop* workshop;
    Role role;
    int accountId;
    QString userName;
    int masterId;
    bool relogin = false;
    bool darkTheme = false;
    bool autoResizing = false;
    bool userSized[6] = {false, false, false, false, false, false};
    QString lastDate;                             
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
