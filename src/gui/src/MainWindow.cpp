#include "MainWindow.h"

#include <algorithm>
#include <functional>

#include <QAction>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableView>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "ClientDialog.h"
#include "ClientTableModel.h"
#include "Dates.h"
#include "LogTableModel.h"
#include "DetailsDialog.h"
#include "MasterDialog.h"
#include "MasterTableModel.h"
#include "OrderDialog.h"
#include "OrderTableModel.h"
#include "PartDialog.h"
#include "PartTableModel.h"
#include "SortProxy.h"
#include "StatusDelegate.h"
#include "Style.h"
#include "WatchDialog.h"
#include "WatchTableModel.h"
#include "Workshop.h"

namespace {
// порядок кнопок боковой панели: заказы, клиенты, часы, мастера, детали
const int kNavOrder[6] = {3, 0, 1, 2, 4, 5};
const char* const kPageTitles[6] = {"Клиенты", "Часы", "Мастера", "Заказы", "Детали на складе", "Журнал действий"};
const char* const kNavTitles[6] = {"Клиенты", "Часы", "Мастера", "Заказы", "Детали", "Журнал"};
const char* const kAddTitles[6] = {"+ Клиент", "+ Часы", "+ Мастер", "+ Заказ", "+ Деталь", ""};
const int kMaxColumnWidth = 340;       // длинный текст дальше обрезается, полный виден в подсказке
} // namespace

template <typename F>
void MainWindow::guarded(F&& action) {
    try {
        action();
    } catch (const WorkshopException& e) {
        showError(e);
    }
}

MainWindow::MainWindow(Workshop* workshop, Role role, int accountId, const QString& userName,
                       int masterId, QWidget* parent)
    : QMainWindow(parent), workshop(workshop), role(role), accountId(accountId), userName(userName),
      masterId(masterId) {
    setWindowTitle("Часовая мастерская — " + userName + " (" +
                   QString::fromStdString(roleToString(role)) + ")");
    resize(1250, 700);
    darkTheme = QSettings("WatchWorkshop", "WatchWorkshop").value("darkTheme", false).toBool();

    clientModel = new ClientTableModel(workshop->clients, this);
    watchModel = new WatchTableModel(workshop->watches, this);
    masterModel = new MasterTableModel(workshop->masters, this);
    orderModel = new OrderTableModel(workshop->orders, this);
    partModel = new PartTableModel(workshop->parts, this);
    logModel = new LogTableModel(workshop->actionLog, this);

    buildUi();
    createMenus();
    createShortcuts();
    for (int i = 0; i < 6; ++i) fitColumns(static_cast<Tab>(i));

    if (allowed(Staff)) {
        QString path = QString::fromStdString(workshop->getDataDir());
        statusBar()->showMessage("Данные сохраняются в папку: " + QDir::toNativeSeparators(path));
    }

    // сроки зависят от текущей даты: при её смене таблица заказов обновляется сама
    lastDate = QString::fromStdString(dates::today());
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this] {
        QString today = QString::fromStdString(dates::today());
        if (today != lastDate) {
            lastDate = today;
            refreshTab(OrdersTab);
        }
    });
    timer->start(60 * 1000);
}

// ---------- оформление окна ----------

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // боковая панель
    auto* sidebar = new QFrame(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(210);
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(0);
    auto* appTitle = new QLabel("Часовая\nмастерская", sidebar);
    appTitle->setObjectName("appTitle");
    sideLayout->addWidget(appTitle);

    auto* group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < 6; ++i) {
        Tab tab = static_cast<Tab>(kNavOrder[i]);
        auto* button = new QPushButton(kNavTitles[tab], sidebar);
        button->setObjectName("nav");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setVisible(pageVisible(tab));
        group->addButton(button);
        sideLayout->addWidget(button);
        navButtons[tab] = button;
        connect(button, &QPushButton::clicked, this, [this, tab] { setPage(tab); });
    }
    sideLayout->addStretch();
    auto* userLabel = new QLabel(userName + "\n" + QString::fromStdString(roleToString(role)), sidebar);
    userLabel->setObjectName("userLabel");
    sideLayout->addWidget(userLabel);

    // рабочая область
    auto* content = new QWidget(central);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(22, 18, 22, 14);
    contentLayout->setSpacing(12);

    auto* header = new QHBoxLayout;
    titleLabel = new QLabel(content);
    titleLabel->setObjectName("pageTitle");
    header->addWidget(titleLabel);
    header->addStretch();
    searchEdit = new QLineEdit(content);
    searchEdit->setPlaceholderText("Поиск (Ctrl+F)");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setFixedWidth(260);
    header->addWidget(searchEdit);
    takeButton = new QPushButton("Взять заказ", content);
    takeButton->setObjectName("takeButton");
    takeButton->setCursor(Qt::PointingHandCursor);
    header->addWidget(takeButton);
    addButton = new QPushButton(content);
    addButton->setObjectName("addButton");
    addButton->setCursor(Qt::PointingHandCursor);
    header->addWidget(addButton);
    contentLayout->addLayout(header);

    cardsBar = new QWidget(content);
    auto* cardsLayout = new QHBoxLayout(cardsBar);
    cardsLayout->setContentsMargins(0, 0, 0, 0);
    cardsLayout->setSpacing(12);
    cardsLayout->addWidget(makeCard("dark", "Всего заказов", cardValues[0]), 1);
    cardsLayout->addWidget(makeCard("green", "В работе", cardValues[1]), 1);
    cardsLayout->addWidget(makeCard("red", "Просрочено", cardValues[2]), 1);
    cardsLayout->addWidget(makeCard("orange", "Готовы к выдаче", cardValues[3]), 1);
    contentLayout->addWidget(cardsBar);

    pages = new QStackedWidget(content);
    contentLayout->addWidget(pages, 1);
    tables[ClientsTab] = addTable(clientModel, ClientsTab);
    tables[WatchesTab] = addTable(watchModel, WatchesTab);
    tables[MastersTab] = addTable(masterModel, MastersTab);
    tables[OrdersTab] = addTable(orderModel, OrdersTab);
    tables[PartsTab] = addTable(partModel, PartsTab);
    tables[LogTab] = addTable(logModel, LogTab);
    proxies[LogTab]->toggleColumn(0);                 // журнал: сначала новые записи

    root->addWidget(sidebar);
    root->addWidget(content, 1);
    setCentralWidget(central);

    connect(searchEdit, &QLineEdit::textChanged, this,
            [this](const QString& text) { proxies[currentTab()]->setFilterFixedString(text); });
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddCurrent);
    connect(takeButton, &QPushButton::clicked, this, &MainWindow::onTakeOrder);

    navButtons[OrdersTab]->setChecked(true);
    setPage(OrdersTab);
    updateSummary();
}

QWidget* MainWindow::makeCard(const QString& tone, const QString& caption, QLabel*& valueLabel) {
    auto* card = new QFrame;
    card->setObjectName("card");
    card->setProperty("tone", tone);
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(0);
    valueLabel = new QLabel("0", card);
    valueLabel->setObjectName("cardValue");
    auto* captionLabel = new QLabel(caption, card);
    captionLabel->setObjectName("cardCaption");
    layout->addWidget(valueLabel);
    layout->addWidget(captionLabel);
    return card;
}

QTableView* MainWindow::addTable(QAbstractItemModel* model, Tab tab) {
    auto* proxy = new SortProxy(this);          // поиск по любому столбцу и сортировка по щелчку на заголовок
    proxy->setSource(model);
    proxies[tab] = proxy;

    auto* view = new QTableView(pages);
    view->setModel(proxy);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAlternatingRowColors(true);
    view->setShowGrid(false);
    view->setWordWrap(false);
    view->verticalHeader()->hide();
    view->verticalHeader()->setDefaultSectionSize(40);

    QHeaderView* header = view->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);     // ширину столбцов можно менять мышью
    header->setMinimumSectionSize(60);
    header->setStretchLastSection(true);
    header->setSectionsClickable(true);
    connect(header, &QHeaderView::sectionClicked, this,
            [this, tab](int column) { proxies[tab]->toggleColumn(column); });
    connect(header, &QHeaderView::sectionResized, this, [this, tab] {
        if (!autoResizing) userSized[tab] = true;               // пользователь сам задал ширину
    });

    view->setContextMenuPolicy(Qt::CustomContextMenu);          // меню по правой кнопке мыши
    connect(view, &QWidget::customContextMenuRequested, this,
            [this, tab](const QPoint& pos) { showContextMenu(tab, pos); });
    connect(view, &QTableView::doubleClicked, this, [this, tab](const QModelIndex&) {
        if (tab == OrdersTab || tab == WatchesTab) showDetails(tab);
    });
    if (tab == OrdersTab) view->setItemDelegateForColumn(5, new StatusDelegate(view));
    pages->addWidget(view);               // индекс страницы совпадает с номером раздела
    return view;
}

// Ширина столбцов подгоняется под содержимое, пока пользователь не изменил её вручную.
void MainWindow::fitColumns(Tab tab) {
    if (userSized[tab]) return;
    autoResizing = true;
    QTableView* view = tables[tab];
    view->resizeColumnsToContents();
    for (int c = 0; c < view->model()->columnCount(); ++c)
        if (view->columnWidth(c) > kMaxColumnWidth) view->setColumnWidth(c, kMaxColumnWidth);
    autoResizing = false;
}

void MainWindow::setPage(Tab tab) {
    pages->setCurrentIndex(tab);
    for (SortProxy* proxy : proxies) proxy->setFilterFixedString("");
    searchEdit->clear();
    titleLabel->setText(kPageTitles[tab]);
    cardsBar->setVisible(tab == OrdersTab);
    updateAddButton();
}

void MainWindow::updateAddButton() {
    Tab tab = currentTab();
    addButton->setText(kAddTitles[tab]);
    addButton->setVisible(allowed(addPermission(tab)));
    takeButton->setVisible(tab == OrdersTab && role == Role::Master);   // мастер берёт заказы сам
}

// Клиентов, часы, заказы и детали добавляет ресепшен (и администратор); мастеров - администратор.
int MainWindow::addPermission(Tab tab) const {
    if (tab == LogTab) return 0;                       // в журнал записи добавляются сами
    return tab == MastersTab ? AdminOnly : FrontDesk;
}

// Какие разделы видит роль: гость и мастер - заказы, часы и детали;
// ресепшен и администратор - все.
bool MainWindow::pageVisible(Tab tab) const {
    if (tab == LogTab) return role == Role::Admin;     // журнал действий видит только администратор
    if (role == Role::Reception || role == Role::Admin) return true;
    return tab == OrdersTab || tab == WatchesTab || tab == PartsTab;
}

void MainWindow::updateSummary() {
    int total = 0, working = 0, overdue = 0, ready = 0;
    std::string today = dates::today();
    for (const Order& o : workshop->orders) {
        ++total;
        if (o.getStatus() == OrderStatus::Diagnostics || o.getStatus() == OrderStatus::InProgress) ++working;
        if (o.getStatus() == OrderStatus::Ready) ++ready;
        if (o.getStatus() < OrderStatus::Ready && o.daysLeft(today) < 0) ++overdue;
    }
    cardValues[0]->setText(QString::number(total));
    cardValues[1]->setText(QString::number(working));
    cardValues[2]->setText(QString::number(overdue));
    cardValues[3]->setText(QString::number(ready));
}

MainWindow::Tab MainWindow::currentTab() const { return static_cast<Tab>(pages->currentIndex()); }

// ---------- меню, горячие клавиши и права ----------

bool MainWindow::allowed(int who) const {
    int bit = 0;
    switch (role) {
        case Role::Guest:     bit = ForGuest; break;
        case Role::Master:    bit = ForMaster; break;
        case Role::Reception: bit = ForReception; break;
        case Role::Admin:     bit = ForAdmin; break;
    }
    return (who & bit) != 0;
}

// Мастер меняет заказ только если взял его сам.
bool MainWindow::ownsOrder(int orderId) const {
    if (role != Role::Master) return false;
    const Master* m = workshop->orders.findById(orderId).getMaster();
    return m && m->getId() == masterId;
}

QMenu* MainWindow::newMenu(const QString& title) { return new QMenu(title, this); }

void MainWindow::attach(QMenu* menu) {
    if (menu->isEmpty()) delete menu;           // у роли нет ни одного действия в этом меню
    else menuBar()->addMenu(menu);
}

void MainWindow::addMenuAction(QMenu* menu, const QString& title, void (MainWindow::*slot)(), int who,
                               const QKeySequence& shortcut) {
    if (!allowed(who)) return;
    QAction* action = menu->addAction(title);
    if (!shortcut.isEmpty()) action->setShortcut(shortcut);
    connect(action, &QAction::triggered, this, slot);
}

// В верхней панели остались только общие действия: все операции с данными выполняются
// внутри самих разделов (кнопка «+», двойной щелчок, правая кнопка мыши).
void MainWindow::createMenus() {
    QMenu* file = newMenu("Файл");
    if (allowed(Staff)) {
        QAction* save = file->addAction("Сохранить");
        save->setShortcut(QKeySequence::Save);                       // Ctrl+S
        connect(save, &QAction::triggered, this, [this] {
            guarded([this] {
                workshop->saveAll();
                statusBar()->showMessage("Данные сохранены", 3000);
            });
        });
    }
    addMenuAction(file, "Открыть папку с данными", &MainWindow::onOpenDataFolder, Staff);
    addMenuAction(file, "Сменить пароль…", &MainWindow::onChangePassword, Staff);
    file->addSeparator();
    addMenuAction(file, "Сменить тему (светлая / тёмная)", &MainWindow::onToggleTheme, Everyone,
                  QKeySequence("Ctrl+D"));
    addMenuAction(file, "Сменить пользователя", &MainWindow::onSwitchUser, Everyone, QKeySequence("Ctrl+L"));
    QAction* quit = file->addAction("Выход");
    quit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quit, &QAction::triggered, this, &QWidget::close);
    attach(file);

    QMenu* staff = newMenu("Сотрудники");
    addMenuAction(staff, "Создать учётную запись…", &MainWindow::onCreateAccount, AdminOnly);
    addMenuAction(staff, "Удалить учётную запись…", &MainWindow::onRemoveAccount, AdminOnly);
    attach(staff);
}

// Горячие клавиши без пунктов меню. Все они действуют только в текущем разделе
// и проверяют права так же, как кнопки и контекстные меню.
void MainWindow::createShortcuts() {
    auto bind = [this](const QKeySequence& keys, std::function<void()> action) {
        auto* a = new QAction(this);
        a->setShortcut(keys);
        connect(a, &QAction::triggered, this, [action] { action(); });
        QWidget::addAction(a);
    };
    bind(QKeySequence::New, [this] { onAddCurrent(); });                        // Ctrl+N
    bind(QKeySequence(Qt::Key_F2), [this] { onEdit(); });
    bind(QKeySequence::Delete, [this] { onDelete(); });
    bind(QKeySequence("Ctrl+T"), [this] { if (currentTab() == OrdersTab) onChangeStatus(); });
    bind(QKeySequence::Find, [this] {                                           // Ctrl+F
        searchEdit->setFocus();
        searchEdit->selectAll();
    });
    bind(QKeySequence(Qt::Key_F5), [this] { refreshAll(); });
    for (int i = 0; i < 6; ++i) {                                               // Ctrl+1 ... Ctrl+6
        Tab tab = static_cast<Tab>(kNavOrder[i]);
        bind(QKeySequence(QString("Ctrl+%1").arg(i + 1)), [this, tab] {
            if (!navButtons[tab]->isVisible()) return;
            navButtons[tab]->setChecked(true);
            setPage(tab);
        });
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (role != Role::Guest)
        guarded([this] { workshop->saveAll(); });     // автосохранение при выходе
    event->accept();
}

void MainWindow::showError(const WorkshopException& e) {
    QMessageBox::warning(this, "Ошибка", QString::fromUtf8(e.what()));
}

void MainWindow::refreshTab(Tab tab) {
    switch (tab) {
        case ClientsTab: clientModel->refresh(); break;
        case WatchesTab: watchModel->refresh(); break;
        case MastersTab: masterModel->refresh(); break;
        case OrdersTab:  orderModel->refresh(); updateSummary(); break;
        case PartsTab:   partModel->refresh(); break;
        case LogTab:     logModel->refresh(); break;
    }
    fitColumns(tab);
}

void MainWindow::refreshAll() {
    for (int i = 0; i < 6; ++i) refreshTab(static_cast<Tab>(i));
}

// ---------- вспомогательные функции ----------

// id выбранной строки: идентификатор всегда лежит в столбце 0.
int MainWindow::selectedId(Tab tab) const {
    QModelIndex index = tables[tab]->currentIndex();
    if (!index.isValid()) throw InvalidInputException("не выбрана строка в таблице");
    return index.sibling(index.row(), 0).data().toInt();
}

bool MainWindow::pick(const QString& title, const Choices& choices, int& id) {
    if (choices.isEmpty()) throw InvalidInputException("список для выбора пуст");
    QStringList titles;
    for (const Choice& c : choices) titles << c.title;
    bool ok = false;
    QString selected = QInputDialog::getItem(this, title, "Выберите:", titles, 0, false, &ok);
    if (!ok) return false;
    id = choices[static_cast<int>(titles.indexOf(selected))].id;
    return true;
}

Choices MainWindow::clientChoices() const {
    Choices r;
    for (const Client& c : workshop->clients)
        r.push_back({c.getId(), QString::fromStdString(c.getLastName() + " " + c.getFirstName()), 0, "", ""});
    return r;
}

Choices MainWindow::watchChoices(bool freeOnly) const {
    Choices r;
    for (const Watch& w : workshop->watches) {
        if (freeOnly && workshop->activeOrderId(w.getId()) != 0) continue;   // на часы уже оформлен заказ
        r.push_back({w.getId(),
                     QString::fromStdString(w.getBrand() + " " + w.getModel() + " (" + w.getSerialNumber() + ")"),
                     w.getClientId(), QString::fromStdString(w.getMechanismType()),
                     QString::fromStdString(w.getBrand())});
    }
    return r;
}

Choices MainWindow::masterChoices() const {
    Choices r;
    for (const Master& m : workshop->masters)
        r.push_back({m.getId(), QString::fromStdString(m.getLastName() + " " + m.getFirstName()), 0, "", ""});
    return r;
}

Choices MainWindow::partChoices() const {
    Choices r;
    for (const Part& p : workshop->parts)
        r.push_back({p.getId(),
                     QString::fromStdString(p.getName()) + " (остаток " + QString::number(p.getQuantity()) + ")",
                     0, "", ""});
    return r;
}

Choices MainWindow::orderChoices(bool ownOnly) const {
    Choices r;
    for (const Order& o : workshop->orders) {
        if (o.getStatus() >= OrderStatus::Ready) continue;        // готовые и выданные заказы не меняются
        if (ownOnly && (!o.getMaster() || o.getMaster()->getId() != masterId)) continue;
        QString watch = o.getWatch() ? QString::fromStdString(o.getWatch()->getBrand() + " " + o.getWatch()->getModel())
                                     : QString("—");
        QString client = o.getClient() ? QString::fromStdString(o.getClient()->getLastName()) : QString("—");
        r.push_back({o.getId(), "№" + QString::number(o.getId()) + ": " + watch + ", " + client, 0, "", ""});
    }
    return r;
}

Choices MainWindow::accountChoices() const {
    Choices r;
    for (const Account& a : workshop->accounts)
        r.push_back({a.getId(),
                     QString::fromStdString(a.getLogin()) + " (" + QString::fromStdString(roleToString(a.getRole())) + ")",
                     0, "", ""});
    return r;
}

// ---------- подробная информация ----------

QString MainWindow::watchDetails(int watchId) const {
    const Watch& w = workshop->watches.findById(watchId);
    QString html = "<h3>Часы: " + QString::fromStdString(w.getBrand() + " " + w.getModel()).toHtmlEscaped() + "</h3><p>";
    if (role != Role::Guest && w.getClient())                    // гость не видит персональные данные
        html += "<b>Владелец:</b> " +
                QString::fromStdString(w.getClient()->getLastName() + " " + w.getClient()->getFirstName()).toHtmlEscaped() +
                "<br>";
    html += "<b>Серийный номер:</b> " + QString::fromStdString(w.getSerialNumber()).toHtmlEscaped() +
            "<br><b>Тип механизма:</b> " + QString::fromStdString(w.getMechanismType()).toHtmlEscaped() +
            "</p><p><b>Неисправность:</b><br>" +
            QString::fromStdString(w.getDefectDescription()).toHtmlEscaped().replace("\n", "<br>") + "</p>";
    return html;
}

QString MainWindow::orderDetails(int orderId) const {
    const Order& o = workshop->orders.findById(orderId);
    auto esc = [](const std::string& s) { return QString::fromStdString(s).toHtmlEscaped(); };
    QString html = "<h3>Заказ №" + QString::number(o.getId()) + "</h3><p>";
    if (role != Role::Guest && o.getClient())
        html += "<b>Клиент:</b> " + esc(o.getClient()->getLastName() + " " + o.getClient()->getFirstName()) + "<br>";
    if (o.getWatch())
        html += "<b>Часы:</b> " + esc(o.getWatch()->getBrand() + " " + o.getWatch()->getModel() + " (" +
                                       o.getWatch()->getSerialNumber() + ", " + o.getWatch()->getMechanismType() + ")") +
                "<br>";
    html += "<b>Статус:</b> " + esc(statusToString(o.getStatus())) + "<br><b>Мастер:</b> " +
            (o.getMaster() ? esc(o.getMaster()->getLastName() + " " + o.getMaster()->getFirstName()) : QString("не назначен")) +
            "<br><b>Принят:</b> " + esc(o.getAcceptDate()) + "<br><b>Плановая дата:</b> " + esc(o.plannedDate()) +
            "<br><b>Выдан:</b> " + (o.getIssueDate().empty() ? QString("—") : esc(o.getIssueDate())) + "</p>";
    if (o.getWatch())
        html += "<p><b>Неисправность:</b><br>" + esc(o.getWatch()->getDefectDescription()).replace("\n", "<br>") + "</p>";
    html += "<p><b>Услуги:</b></p><ul>";
    for (const auto& s : o.getServices())
        html += "<li>" + esc(s->getName()) + " — " + QString::number(s->calculateCost(), 'f', 2) + " BYN</li>";
    html += "</ul><p><b>Детали:</b></p>";
    if (o.getUsedParts().empty()) html += "<p>нет</p>";
    else {
        html += "<ul>";
        for (const Part* p : o.getUsedParts())
            html += "<li>" + esc(p->getName()) + " — " + QString::number(p->getPrice(), 'f', 2) + " BYN</li>";
        html += "</ul>";
    }
    html += "<p><b>Итого:</b> " + QString::number(o.totalCost(), 'f', 2) + " BYN</p>";
    return html;
}

void MainWindow::showDetails(Tab tab) {
    guarded([this, tab] {
        int id = selectedId(tab);
        QString title = tab == OrdersTab ? "Заказ №" + QString::number(id) : QString("Часы");
        DetailsDialog dlg(title, tab == OrdersTab ? orderDetails(id) : watchDetails(id), this);
        dlg.exec();
    });
}

// ---------- действия, общие для меню и контекстных меню ----------

// Принять часы; владелец (если известен) выбирается в диалоге заранее.
void MainWindow::addWatchFor(int clientId) {
    guarded([this, clientId] {
        WatchDialog dlg(clientChoices(), this, nullptr, clientId);
        if (dlg.exec() != QDialog::Accepted) return;
        workshop->acceptWatch(dlg.getWatch());
        refreshTab(WatchesTab);
    });
}

// Создать заказ; клиент и часы (если известны) выбираются в диалоге заранее.
void MainWindow::createOrderFor(int clientId, int watchId) {
    guarded([this, clientId, watchId] {
        if (watchId != 0)
            if (int active = workshop->activeOrderId(watchId)) {
                QMessageBox::information(this, "Новый заказ",
                    "На эти часы уже оформлен заказ №" + QString::number(active) + ", он ещё не выдан.");
                return;
            }
        OrderDialog dlg(clientChoices(), watchChoices(true), masterChoices(), false, this, clientId, watchId);
        if (dlg.exec() != QDialog::Accepted) return;
        workshop->createOrder(dlg.getClientId(), dlg.getWatchId(), dlg.getMasterId(), dlg.getService());
        refreshTab(OrdersTab);
    });
}

void MainWindow::applyStatus(int orderId, OrderStatus status) {
    guarded([&] {
        workshop->changeStatus(orderId, status);
        refreshTab(OrdersTab);
    });
}

void MainWindow::applyMaster(int orderId, int newMasterId) {
    guarded([&] {
        workshop->assignMaster(orderId, newMasterId);
        refreshTab(OrdersTab);
    });
}

void MainWindow::applyUsedPart(int orderId, int partId) {
    guarded([&] {
        workshop->addUsedPart(orderId, partId);     // деталь сразу списывается со склада
        refreshTab(OrdersTab);
        refreshTab(PartsTab);
    });
}

// ---------- контекстное меню (правая кнопка мыши) ----------

// Меню зависит от раздела, от того, нажата ли строка, от столбца, от состояния заказа и от роли.
// У готового заказа остаётся только смена статуса, у выданного - ничего, кроме «Создать заказ».
void MainWindow::showContextMenu(Tab tab, const QPoint& pos) {
    if (role == Role::Guest) return;
    QTableView* view = tables[tab];
    QModelIndex idx = view->indexAt(pos);
    bool hasRow = idx.isValid();
    if (hasRow) {                                    // правая кнопка выбирает строку, по которой нажали
        view->selectRow(idx.row());
        view->setCurrentIndex(idx);
    }
    int id = hasRow ? idx.sibling(idx.row(), 0).data().toInt() : 0;
    int column = hasRow ? idx.column() : -1;

    QMenu menu(this);
    auto add = [&](const QString& title, int who, std::function<void()> action) {
        if (!allowed(who)) return;
        QAction* a = menu.addAction(title);
        connect(a, &QAction::triggered, this, [action] { action(); });
    };
    auto separator = [&] {
        if (!menu.isEmpty() && !menu.actions().last()->isSeparator()) menu.addSeparator();
    };

    switch (tab) {
        case ClientsTab:
            add("Добавить клиента", FrontDesk, [this] { onAddClient(); });
            if (hasRow) {
                add("Принять часы этого клиента…", FrontDesk, [this, id] { addWatchFor(id); });
                add("Редактировать", FrontDesk, [this] { onEdit(); });
                separator();
                add("Удалить", AdminOnly, [this] { onDelete(); });
            }
            break;

        case WatchesTab:
            add("Принять часы", FrontDesk, [this] { onAddWatch(); });
            if (hasRow) {
                int owner = workshop->watches.findById(id).getClientId();
                add("Подробнее (неисправность)", Staff, [this] { showDetails(WatchesTab); });
                add("Создать заказ на эти часы…", FrontDesk, [this, owner, id] { createOrderFor(owner, id); });
                add("Редактировать", FrontDesk, [this] { onEdit(); });
                separator();
                add("Удалить", AdminOnly, [this] { onDelete(); });
            }
            break;

        case MastersTab:
            add("Добавить мастера", AdminOnly, [this] { onAddMaster(); });
            if (hasRow) {
                if (allowed(FrontDesk)) {            // «Назначить на заказ» выбранного мастера
                    QMenu* sub = menu.addMenu("Назначить на заказ");
                    Choices orders = orderChoices();
                    if (orders.isEmpty()) sub->addAction("Свободных заказов нет")->setEnabled(false);
                    for (const Choice& o : orders) {
                        QAction* action = sub->addAction(o.title);
                        int orderId = o.id;
                        connect(action, &QAction::triggered, this,
                                [this, orderId, id] { applyMaster(orderId, id); });
                    }
                }
                add("Редактировать", AdminOnly, [this] { onEdit(); });
                separator();
                add("Удалить", AdminOnly, [this] { onDelete(); });
            }
            break;

        case PartsTab:
            add("Добавить деталь", FrontDesk, [this] { onAddPart(); });
            if (hasRow) {
                add("Принять партию (пополнить остаток)…", FrontDesk, [this] { onRestockPart(); });
                add("Списать…", WorkshopStaff, [this] { onWriteOffPart(); });
                if (allowed(WorkshopStaff)) {        // «Добавить в заказ» выбранную деталь
                    QMenu* sub = menu.addMenu("Добавить в заказ");
                    Choices orders = orderChoices(role == Role::Master);
                    if (orders.isEmpty()) sub->addAction("Свободных заказов нет")->setEnabled(false);
                    for (const Choice& o : orders) {
                        QAction* action = sub->addAction(o.title);
                        int orderId = o.id;
                        connect(action, &QAction::triggered, this,
                                [this, orderId, id] { applyUsedPart(orderId, id); });
                    }
                }
                separator();
                add("Редактировать", AdminOnly, [this] { onEdit(); });
                add("Удалить", AdminOnly, [this] { onDelete(); });
            }
            break;

        case LogTab:
            break;                                   // у журнала меню нет

        case OrdersTab: {
            const Order* order = hasRow ? &workshop->orders.findById(id) : nullptr;
            bool finished = order && order->getStatus() >= OrderStatus::Ready;
            bool issued = order && order->getStatus() == OrderStatus::Issued;
            bool owns = hasRow && ownsOrder(id);
            bool statusDone = false, masterDone = false, serviceDone = false, partDone = false;

            auto statusMenu = [&] {
                if (statusDone || !hasRow || !allowed(Staff)) return;
                OrderStatus current = order->getStatus();
                // в меню только те статусы, которые эта роль может поставить этому заказу
                // (мастер - только в своём заказе); если таких нет, пункта «Сменить статус» нет вообще
                QList<OrderStatus> options;
                for (int s = 0; s <= static_cast<int>(OrderStatus::Issued); ++s) {
                    OrderStatus status = static_cast<OrderStatus>(s);
                    if (status != current && Workshop::isStatusAllowed(role, current, status) &&
                        (role != Role::Master || owns))
                        options.push_back(status);
                }
                if (options.isEmpty()) return;
                statusDone = true;
                QMenu* sub = menu.addMenu("Сменить статус");
                QAction* now = sub->addAction("Сейчас: " + QString::fromStdString(statusToString(current)));
                now->setEnabled(false);
                sub->addSeparator();
                for (OrderStatus status : options) {
                    QAction* action = sub->addAction(QString::fromStdString(statusToString(status)));
                    connect(action, &QAction::triggered, this,
                            [this, id, status] { applyStatus(id, status); });
                }
            };
            auto masterMenu = [&] {
                if (masterDone || !hasRow || !allowed(FrontDesk)) return;
                masterDone = true;
                QMenu* sub = menu.addMenu("Назначить мастера");
                const Master* current = order->getMaster();
                Choices masters = masterChoices();
                if (masters.isEmpty()) sub->addAction("Мастеров нет")->setEnabled(false);
                for (const Choice& m : masters) {
                    QAction* action = sub->addAction(m.title);
                    action->setCheckable(true);
                    action->setChecked(current && current->getId() == m.id);
                    int newMasterId = m.id;
                    connect(action, &QAction::triggered, this,
                            [this, id, newMasterId] { applyMaster(id, newMasterId); });
                }
            };
            auto serviceMenu = [&] {
                if (serviceDone || !hasRow) return;
                serviceDone = true;
                if (role != Role::Master || owns)             // мастер добавляет услуги только в свой заказ
                    add("Добавить услугу…", Staff, [this] { onAddService(); });
                add("Удалить услугу…", FrontDesk, [this] { onRemoveService(); });
            };
            auto partMenu = [&] {
                if (partDone || !hasRow || !allowed(WorkshopStaff)) return;
                if (role == Role::Master && !owns) return;      // чужой заказ: детали менять нельзя
                partDone = true;
                QMenu* sub = menu.addMenu("Добавить деталь в заказ");
                Choices parts = partChoices();
                if (parts.isEmpty()) sub->addAction("Деталей нет")->setEnabled(false);
                for (const Choice& p : parts) {
                    QAction* action = sub->addAction(p.title);
                    int partId = p.id;
                    connect(action, &QAction::triggered, this,
                            [this, id, partId] { applyUsedPart(id, partId); });
                }
                add("Убрать деталь из заказа…", WorkshopStaff, [this] { onRemoveUsedPart(); });
            };

            if (hasRow) {
                add("Подробнее (неисправность, услуги)", Staff, [this] { showDetails(OrdersTab); });
                separator();
                if (!finished) {
                    // мастер может взять свободный заказ; сложные назначает ресепшен
                    if (role == Role::Master && !order->getMaster()) {
                        QAction* take = menu.addAction(order->isComplex()
                            ? "Взять заказ (сложный: назначает ресепшен)" : "Взять заказ");
                        take->setEnabled(!order->isComplex());
                        connect(take, &QAction::triggered, this, [this] { onTakeOrder(); });
                        separator();
                    }
                    // сначала то, что относится к столбцу, на котором нажата правая кнопка
                    if (column == 5) statusMenu();
                    else if (column == 3) masterMenu();
                    else if (column == 4) serviceMenu();
                    separator();
                    statusMenu();
                    masterMenu();
                    serviceMenu();
                    partMenu();
                } else if (!issued || role == Role::Admin) {
                    statusMenu();                    // готовый заказ: только смена статуса (например, выдача)
                }
                separator();
            }
            add("Создать заказ…", FrontDesk, [this] { onCreateOrder(); });
            if (hasRow && !finished) add("Удалить заказ", AdminOnly, [this] { onDelete(); });
            break;
        }
    }
    if (menu.isEmpty()) return;
    menu.exec(view->viewport()->mapToGlobal(pos));
}

// ---------- добавление, редактирование, удаление ----------

void MainWindow::onAddClient() {
    guarded([this] {
        ClientDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;
        workshop->addClient(dlg.getClient());
        refreshTab(ClientsTab);
    });
}

void MainWindow::onAddWatch() { addWatchFor(0); }

void MainWindow::onAddMaster() {
    guarded([this] {
        MasterDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;
        workshop->addMaster(dlg.getMaster());
        refreshTab(MastersTab);
    });
}

void MainWindow::onAddPart() {
    guarded([this] {
        PartDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;
        bool merged = workshop->addPart(dlg.getPart());
        refreshTab(PartsTab);
        if (merged)
            QMessageBox::information(this, "Приём детали",
                "Такая деталь уже была на складе: количество пополнено на привезённое.");
    });
}

// Ctrl+N и кнопка «+»: добавление в текущем разделе.
void MainWindow::onAddCurrent() {
    Tab tab = currentTab();
    if (!allowed(addPermission(tab))) return;
    switch (tab) {
        case ClientsTab: onAddClient(); break;
        case WatchesTab: onAddWatch(); break;
        case MastersTab: onAddMaster(); break;
        case OrdersTab:  onCreateOrder(); break;
        case PartsTab:   onAddPart(); break;
    }
}

// Редактирование строки текущего раздела: тот же диалог, заполненный выбранным объектом.
// Клиентов и часы редактирует ресепшен, мастеров и детали - администратор.
void MainWindow::onEdit() {
    Tab tab = currentTab();
    if (!allowed((tab == MastersTab || tab == PartsTab) ? AdminOnly : FrontDesk)) return;
    guarded([this, tab] {
        switch (tab) {
            case ClientsTab: {
                int id = selectedId(ClientsTab);
                ClientDialog dlg(this, &workshop->clients.findById(id));
                if (dlg.exec() != QDialog::Accepted) return;
                workshop->updateClient(id, dlg.getClient());
                break;
            }
            case WatchesTab: {
                int id = selectedId(WatchesTab);
                WatchDialog dlg(clientChoices(), this, &workshop->watches.findById(id));
                if (dlg.exec() != QDialog::Accepted) return;
                workshop->updateWatch(id, dlg.getWatch());
                break;
            }
            case MastersTab: {
                int id = selectedId(MastersTab);
                MasterDialog dlg(this, &workshop->masters.findById(id));
                if (dlg.exec() != QDialog::Accepted) return;
                workshop->updateMaster(id, dlg.getMaster());
                break;
            }
            case PartsTab: {
                int id = selectedId(PartsTab);
                PartDialog dlg(this, &workshop->parts.findById(id));
                if (dlg.exec() != QDialog::Accepted) return;
                workshop->updatePart(id, dlg.getPart());
                break;
            }
            case OrdersTab:
                QMessageBox::information(this, "Редактирование",
                    "Заказ не редактируется: используйте правую кнопку мыши по заказу.");
                return;
        }
        refreshAll();      // изменения видны и в связанных таблицах (например, имя клиента в заказах)
    });
}

// Удаление выбранной записи (только администратор). Запись, на которую ссылаются
// другие объекты, удалить нельзя: Workshop генерирует ObjectInUseException.
void MainWindow::onDelete() {
    if (!allowed(AdminOnly)) return;
    guarded([this] {
        Tab tab = currentTab();
        int id = selectedId(tab);
        if (QMessageBox::question(this, "Удаление", "Удалить выбранную запись?") != QMessageBox::Yes)
            return;
        switch (tab) {
            case ClientsTab: workshop->removeClient(id); break;
            case WatchesTab: workshop->removeWatch(id); break;
            case MastersTab: workshop->removeMaster(id); break;
            case OrdersTab:  workshop->removeOrder(id); break;
            case PartsTab:   workshop->removePart(id); break;
        }
        refreshAll();      // у отменённого заказа детали вернулись на склад
    });
}

// ---------- заказы ----------

void MainWindow::onCreateOrder() { createOrderFor(0, 0); }

// Мастер берёт свободный лёгкий заказ сам; сложные назначает ресепшен.
void MainWindow::onTakeOrder() {
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        workshop->takeOrder(orderId);
        refreshTab(OrdersTab);
    });
}

void MainWindow::onAssignMaster() {
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        int newMasterId = 0;
        if (!pick("Назначение мастера", masterChoices(), newMasterId)) return;
        applyMaster(orderId, newMasterId);
    });
}

void MainWindow::onAddService() {
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        const Order& order = workshop->orders.findById(orderId);
        if (!order.getWatch()) throw InvalidInputException("у заказа нет часов");
        const Watch* w = order.getWatch();                  // услуги подбираются по часам заказа
        Choices watches;
        watches.push_back({w->getId(), QString::fromStdString(w->getBrand() + " " + w->getModel()),
                           w->getClientId(), QString::fromStdString(w->getMechanismType()),
                           QString::fromStdString(w->getBrand())});
        OrderDialog dlg({}, watches, {}, true, this);
        if (dlg.exec() != QDialog::Accepted) return;
        workshop->addService(orderId, dlg.getService());
        refreshTab(OrdersTab);
    });
}

void MainWindow::onRemoveService() {
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        Choices services;                                    // id услуги - её номер в заказе
        const auto& list = workshop->orders.findById(orderId).getServices();
        for (std::size_t i = 0; i < list.size(); ++i)
            services.push_back({static_cast<int>(i), QString::fromStdString(list[i]->getName()), 0, "", ""});
        int index = 0;
        if (!pick("Удалить услугу из заказа", services, index)) return;
        workshop->removeService(orderId, static_cast<std::size_t>(index));
        refreshTab(OrdersTab);
    });
}

void MainWindow::onAddUsedPart() {
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        int partId = 0;
        if (!pick("Деталь для заказа", partChoices(), partId)) return;
        applyUsedPart(orderId, partId);
    });
}

void MainWindow::onRemoveUsedPart() {
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        Choices used;                                        // уникальные детали заказа
        for (const Part* p : workshop->orders.findById(orderId).getUsedParts()) {
            bool known = std::any_of(used.begin(), used.end(),
                                     [p](const Choice& c) { return c.id == p->getId(); });
            if (!known) used.push_back({p->getId(), QString::fromStdString(p->getName()), 0, "", ""});
        }
        int partId = 0;
        if (!pick("Убрать деталь из заказа", used, partId)) return;
        workshop->removeUsedPart(orderId, partId);           // деталь вернулась на склад
        refreshTab(OrdersTab);
        refreshTab(PartsTab);
    });
}

// В списке только те статусы, которые разрешены роли для этого заказа.
void MainWindow::onChangeStatus() {
    if (!allowed(Staff)) return;
    guarded([this] {
        int orderId = selectedId(OrdersTab);
        if (role == Role::Master && !ownsOrder(orderId))
            throw AccessDeniedException("статус заказа меняет мастер, который его взял");
        OrderStatus current = workshop->orders.findById(orderId).getStatus();
        QStringList names;
        QList<OrderStatus> statuses;
        for (int s = 0; s <= static_cast<int>(OrderStatus::Issued); ++s) {
            OrderStatus status = static_cast<OrderStatus>(s);
            if (status == current || !Workshop::isStatusAllowed(role, current, status)) continue;
            names << QString::fromStdString(statusToString(status));
            statuses.push_back(status);
        }
        if (names.isEmpty())
            throw AccessDeniedException("для вашей роли у этого заказа нет доступных статусов");
        bool ok = false;
        QString chosen = QInputDialog::getItem(this, "Смена статуса", "Новый статус:", names, 0, false, &ok);
        if (!ok) return;
        applyStatus(orderId, statuses[static_cast<int>(names.indexOf(chosen))]);
    });
}

// ---------- склад ----------

void MainWindow::onRestockPart() {
    guarded([this] {
        int partId = selectedId(PartsTab);
        bool ok = false;
        int qty = QInputDialog::getInt(this, "Приём партии", "Количество:", 1, 1, 100000, 1, &ok);
        if (!ok) return;
        workshop->restockPart(partId, qty);
        refreshTab(PartsTab);
    });
}

void MainWindow::onWriteOffPart() {
    guarded([this] {
        int partId = selectedId(PartsTab);
        bool ok = false;
        int qty = QInputDialog::getInt(this, "Списание", "Количество:", 1, 1, 100000, 1, &ok);
        if (!ok) return;
        workshop->writeOffPart(partId, qty);
        refreshTab(PartsTab);
    });
}

// ---------- учётные записи ----------

void MainWindow::onCreateAccount() {
    guarded([this] {
        bool ok = false;
        QString login = QInputDialog::getText(this, "Новая учётная запись",
            "Логин (латинские буквы, цифры, _):", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        QString password = QInputDialog::getText(this, "Новая учётная запись",
            "Пароль (не короче 4 символов):", QLineEdit::Password, "", &ok);
        if (!ok) return;
        QStringList roles{"Мастер", "Ресепшен", "Администратор"};
        QString chosen = QInputDialog::getItem(this, "Новая учётная запись", "Роль:", roles, 0, false, &ok);
        if (!ok) return;
        Role newRole = (chosen == "Мастер") ? Role::Master
                     : (chosen == "Ресепшен") ? Role::Reception : Role::Admin;
        int newMasterId = 0;
        if (newRole == Role::Master && !pick("Мастер для учётной записи", masterChoices(), newMasterId))
            return;
        workshop->addAccount(login.toStdString(), password.toStdString(), newRole, newMasterId);
        QMessageBox::information(this, "Учётная запись", "Учётная запись создана.");
    });
}

void MainWindow::onRemoveAccount() {
    guarded([this] {
        int id = 0;
        if (!pick("Удалить учётную запись", accountChoices(), id)) return;
        if (id == accountId) throw InvalidInputException("нельзя удалить собственную учётную запись");
        workshop->removeAccount(id);
    });
}

void MainWindow::onChangePassword() {
    guarded([this] {
        bool ok = false;
        QString password = QInputDialog::getText(this, "Смена пароля",
            "Новый пароль (не короче 4 символов):", QLineEdit::Password, "", &ok);
        if (!ok) return;
        workshop->changePassword(accountId, password.toStdString());
        QMessageBox::information(this, "Смена пароля", "Пароль изменён.");
    });
}

void MainWindow::onSwitchUser() {
    relogin = true;
    close();
}

// Светлая и тёмная темы; выбор запоминается между запусками.
void MainWindow::onToggleTheme() {
    darkTheme = !darkTheme;
    applyTheme(darkTheme);
    QSettings("WatchWorkshop", "WatchWorkshop").setValue("darkTheme", darkTheme);
}

// Данные (включая учётные записи с хешами паролей) лежат в папке data рядом с программой.
void MainWindow::onOpenDataFolder() {
    QString path = QString::fromStdString(workshop->getDataDir());
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
