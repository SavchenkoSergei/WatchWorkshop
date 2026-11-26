#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>

#include "Exceptions.h"
#include "LoginDialog.h"
#include "MainWindow.h"
#include "Style.h"
#include "Workshop.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");              
    applyTheme(QSettings("WatchWorkshop", "WatchWorkshop").value("darkTheme", false).toBool());

    Workshop workshop((QCoreApplication::applicationDirPath() + "/data").toStdString());
    try {
        workshop.loadAll();
    } catch (const WorkshopException& e) {
        QMessageBox::warning(nullptr, "Загрузка данных",
            QString::fromUtf8(e.what()) +
            "\nФайлы данных переименованы в *.bak. Программа продолжит работу с пустыми данными.");
    }
    workshop.ensureDefaultAccount();       

    while (true) {
        LoginDialog login;
        if (login.exec() != QDialog::Accepted) break;

        Role role = Role::Guest;
        int accountId = 0;
        int masterId = 0;
        QString name = "Гость";
        if (!login.isGuest()) {
            try {
                Account account = workshop.login(login.getLogin().toStdString(),
                                                 login.getPassword().toStdString());
                role = account.getRole();
                accountId = account.getId();
                masterId = account.getMasterId();         
                name = login.getLogin();
            } catch (const WorkshopException& e) {
                QMessageBox::warning(nullptr, "Вход", QString::fromUtf8(e.what()));
                continue;
            }
        }

        workshop.setActor(role == Role::Guest ? "гость" : name.toStdString(), role, masterId);
        MainWindow window(&workshop, role, accountId, name, masterId);
        window.show();
        app.exec();
        if (!window.wantsRelogin()) break;
    }
    return 0;
}
