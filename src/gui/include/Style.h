#pragma once
// Светлая и тёмная темы оформления (QSS). Цвета подставляются в общий шаблон.
#include <QApplication>
#include <QString>

static const char* const kStyleTemplate = R"QSS(
QMainWindow, QDialog { background: @bg@; }
QWidget { font-size: 13px; color: @text@; }
QMenuBar { background: @panel@; border-bottom: 1px solid @line@; }
QMenuBar::item { padding: 6px 12px; background: transparent; }
QMenuBar::item:selected { background: @hover@; }
QMenu { background: @panel@; border: 1px solid @border@; padding: 4px 0; }
QMenu::item { padding: 7px 26px 7px 22px; }
QMenu::item:selected { background: @sel@; color: @selText@; }
QMenu::item:disabled { color: @muted@; }
QMenu::separator { height: 1px; background: @line@; margin: 4px 8px; }
QStatusBar { background: @panel@; color: @muted@; border-top: 1px solid @line@; }

#sidebar { background: @sidebar@; }
#sidebar QLabel, #sidebar QPushButton { color: #c9d1db; }
#appTitle { color: white; font-size: 16px; font-weight: bold; padding: 20px 18px 14px 18px; }
#userLabel { color: #aeb8c4; font-size: 12px; padding: 12px 18px; }
QPushButton#nav { text-align: left; padding: 13px 18px; color: #c9d1db; background: transparent;
                  border: none; border-left: 3px solid transparent; font-size: 14px; }
QPushButton#nav:hover { background: @navHover@; }
QPushButton#nav:checked { background: @navChecked@; color: white; border-left: 3px solid #2ecc71; }

#pageTitle { font-size: 22px; font-weight: bold; color: @text@; }
QPushButton#addButton { background: #2ecc71; color: white; border: none; border-radius: 4px;
                        padding: 9px 18px; font-weight: bold; }
QPushButton#addButton:hover { background: #27ae60; }
QPushButton#takeButton { background: #3498db; color: white; border: none; border-radius: 4px;
                         padding: 9px 18px; font-weight: bold; }
QPushButton#takeButton:hover { background: #2a82bf; }

QLineEdit { background: @panel@; border: 1px solid @border@; border-radius: 4px; padding: 7px 9px;
            selection-background-color: @sel@; selection-color: @selText@; }
QLineEdit:focus { border: 1px solid #3498db; }
QAbstractSpinBox QLineEdit { border: none; padding: 0; background: transparent; }

QFrame#card { border-radius: 5px; }
QFrame#card[tone="green"] { background: #27ae60; }
QFrame#card[tone="red"] { background: #e74c3c; }
QFrame#card[tone="orange"] { background: #f39c12; }
QFrame#card[tone="dark"] { background: #3b4553; }
QLabel#cardValue { color: white; font-size: 24px; font-weight: bold; }
QLabel#cardCaption { color: white; font-size: 12px; }

QTableView { background: @panel@; alternate-background-color: @alt@; border: 1px solid @line@;
             selection-background-color: @sel@; selection-color: @selText@; outline: none; }
QTableView::item { padding: 4px 6px; border-bottom: 1px solid @line@; }
QHeaderView::section { background: @header@; color: @headerText@; border: none;
                       border-right: 1px solid @line@; border-bottom: 1px solid @border@;
                       padding: 9px 8px; font-weight: bold; }
QTableCornerButton::section { background: @header@; border: none; }
QTextBrowser { background: @panel@; border: 1px solid @border@; border-radius: 4px; }

QComboBox, QSpinBox, QDoubleSpinBox, QPlainTextEdit { background: @panel@; border: 1px solid @border@;
                                      border-radius: 4px; padding: 5px; }
QComboBox { padding-right: 26px; }
QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 24px;
                       border-left: 1px solid @border@; }
QComboBox::down-arrow { image: url(@arrowDown@); width: 12px; height: 8px; }
QComboBox QAbstractItemView { background: @panel@; color: @text@; border: 1px solid @border@;
                              selection-background-color: @sel@; selection-color: @selText@; outline: none; }
QSpinBox, QDoubleSpinBox { padding-right: 24px; }
QSpinBox::up-button, QDoubleSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 22px;
                      border-left: 1px solid @border@; border-bottom: 1px solid @border@;
                      border-top-right-radius: 4px; background: @header@; }
QSpinBox::down-button, QDoubleSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 22px;
                        border-left: 1px solid @border@; border-bottom-right-radius: 4px; background: @header@; }
QSpinBox::up-button:hover, QSpinBox::down-button:hover,
QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover { background: @hover@; }
QSpinBox::up-arrow, QDoubleSpinBox::up-arrow { image: url(@arrowUp@); width: 12px; height: 8px; }
QSpinBox::down-arrow, QDoubleSpinBox::down-arrow { image: url(@arrowDown@); width: 12px; height: 8px; }

QDialog QPushButton { background: @panel@; border: 1px solid @border@; border-radius: 4px; padding: 7px 16px; }
QDialog QPushButton:hover { background: @hover@; }
QDialog QPushButton:default { background: #3498db; color: white; border: none; }
QToolTip { background: @sidebar@; color: white; border: none; padding: 4px; }
)QSS";

inline QString buildStyle(bool dark) {
    struct Color { const char* key; const char* light; const char* dark; };
    static const Color colors[] = {
        {"@bg@", "#f3f5f8", "#1e232b"},
        {"@panel@", "#ffffff", "#262c36"},
        {"@text@", "#2c3e50", "#e3e8ee"},
        {"@muted@", "#6b7a8a", "#8793a1"},
        {"@border@", "#cfd6de", "#3a4452"},
        {"@line@", "#e1e6ec", "#313a47"},
        {"@alt@", "#f7f9fb", "#2a313c"},
        {"@header@", "#eef1f5", "#303846"},
        {"@headerText@", "#4a5a6a", "#b9c3cf"},
        {"@sel@", "#dbeafe", "#35507a"},
        {"@selText@", "#1f2d3d", "#ffffff"},
        {"@hover@", "#e8eef5", "#323b48"},
        {"@sidebar@", "#2b3441", "#161a20"},
        {"@navHover@", "#343f4e", "#20262e"},
        {"@navChecked@", "#3a4657", "#2a323d"},
        {"@arrowUp@", ":/resources/arrow_up_dark.png", ":/resources/arrow_up_light.png"},
        {"@arrowDown@", ":/resources/arrow_down_dark.png", ":/resources/arrow_down_light.png"},
    };
    QString qss = QString::fromUtf8(kStyleTemplate);
    for (const Color& c : colors) qss.replace(QString(c.key), QString(dark ? c.dark : c.light));
    return qss;
}

inline void applyTheme(bool dark) { qApp->setStyleSheet(buildStyle(dark)); }
