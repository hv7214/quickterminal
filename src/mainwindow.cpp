/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
** Copyright (C) 2010-2014 Petr Vanek <petr@scribus.info>
** Copyright (C) 2006 by Vladimir Kuznetsov <vovanec@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "mainwindow.h"

#include "actionmanager.h"
#include "constants.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "termwidgetholder.h"
#include "tabwidget.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolButton>

MainWindow::MainWindow(const QString &workingDir, const QString &command, QWidget *parent,
                       Qt::WindowFlags f) :
    QMainWindow(parent, f),
    m_preferences(Preferences::instance()),
    m_actionManager(new ActionManager(this))
{
    /// TODO: Check why it is not set by default
    setAttribute(Qt::WA_DeleteOnClose);

    connect(m_preferences, &Preferences::changed, this, &MainWindow::preferencesChanged);

    m_tabWidget = new TabWidget(this);
    connect(m_tabWidget, &TabWidget::lastTabClosed, this, &MainWindow::close);

    m_tabWidget->tabBar()->setVisible(!m_preferences->hideTabBar);
    m_tabWidget->setWorkDirectory(workingDir);
    m_tabWidget->setTabPosition((QTabWidget::TabPosition)m_preferences->tabBarPosition);
    m_tabWidget->addNewTab(command);
    setCentralWidget(m_tabWidget);

    setWindowTitle(qApp->applicationName());
    setWindowIcon(QIcon::fromTheme(QStringLiteral("utilities-terminal"), QIcon(Icon::Application)));

    setupFileMenu();
    setupEditMenu();
    setupViewMenu();
    setupHelpMenu();
    setupContextMenu();
    setupWindowActions();

    setContentsMargins(0, 0, 0, 0);
    if (!restoreGeometry(m_preferences->mainWindowGeometry))
        resize(800, 600);
    restoreState(m_preferences->mainWindowState);
}

void MainWindow::enableDropMode()
{
    m_dropDownMode = true;
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    setStyleSheet(QStringLiteral("MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"));

    m_dropDownLockButton = new QToolButton(this);
    m_tabWidget->setCornerWidget(m_dropDownLockButton, Qt::BottomRightCorner);
    m_dropDownLockButton->setCheckable(true);
    connect(m_dropDownLockButton, &QToolButton::clicked, this, &MainWindow::setKeepOpen);
    setKeepOpen(m_preferences->dropKeepOpen);
    m_dropDownLockButton->setAutoRaise(true);
    realign();
}

void MainWindow::setupFileMenu()
{
    QMenu *menu = new QMenu(tr("&File"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::NewTab);
    connect(action, SIGNAL(triggered()), m_tabWidget, SLOT(addNewTab()));
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::CloseTab);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::removeCurrentTab);
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::NewWindow);
    connect(action, &QAction::triggered, this, &MainWindow::newWindow);
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::CloseWindow);
    connect(action, &QAction::triggered, this, &MainWindow::close);
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Exit);
    connect(action, &QAction::triggered, this, &MainWindow::quit);
    addAction(action);
    menu->addAction(action);

    menuBar()->addMenu(menu);
}

void MainWindow::setupEditMenu()
{
    QMenu *menu = new QMenu(tr("&Edit"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::Copy);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->copyClipboard();
    });
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::Paste);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->pasteClipboard();
    });
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::PasteSelection);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->pasteSelection();
    });
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Clear);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->clear();
    });
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Find);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->toggleShowSearchBar();
    });
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Preferences);
    connect(action, &QAction::triggered, this, &MainWindow::showPreferencesDialog);
    addAction(action);
    menu->addAction(action);

    menuBar()->addMenu(menu);
}

void MainWindow::setupViewMenu()
{
    QMenu *menu = new QMenu(tr("&View"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::ShowMenu);
    action->setCheckable(true);
    action->setChecked(m_preferences->menuVisible);
    connect(action, &QAction::triggered, this, &MainWindow::toggleMenuBar);
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::ShowTabs);
    action->setCheckable(true);
    action->setChecked(!m_preferences->hideTabBar);
    connect(action, &QAction::triggered, this, &MainWindow::toggleTabBar);
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    // TabBar position
    QActionGroup *tabBarPosition = new QActionGroup(this);
    tabBarPosition->addAction(tr("Top"));
    tabBarPosition->addAction(tr("Bottom"));
    tabBarPosition->addAction(tr("Left"));
    tabBarPosition->addAction(tr("Right"));

    foreach (QAction *action, tabBarPosition->actions())
        action->setCheckable(true);

    if (tabBarPosition->actions().count() > m_preferences->tabBarPosition)
        tabBarPosition->actions().at(m_preferences->tabBarPosition)->setChecked(true);

    connect(tabBarPosition, &QActionGroup::triggered,
            m_tabWidget, &TabWidget::changeTabPosition);

    QMenu *tabBarPositionMenu = new QMenu(tr("Tabs Layout"), menu);
    tabBarPositionMenu->addActions(tabBarPosition->actions());

    QAction *tabBarPositionMenuAction = menu->addMenu(tabBarPositionMenu);
    connect(tabBarPositionMenuAction, &QAction::hovered, [=]() {
        tabBarPosition->actions().at(m_preferences->tabBarPosition)->setChecked(true);
    });

    // Scrollbar position
    QActionGroup *scrollBarPosition = new QActionGroup(this);
    // Order is based on QTermWidget::ScrollBarPosition enum
    scrollBarPosition->addAction(tr("None"));
    scrollBarPosition->addAction(tr("Left"));
    scrollBarPosition->addAction(tr("Rigth"));

    foreach (QAction *action, scrollBarPosition->actions())
        action->setCheckable(true);

    if (m_preferences->scrollBarPosition < scrollBarPosition->actions().size())
        scrollBarPosition->actions().at(m_preferences->scrollBarPosition)->setChecked(true);

    connect(scrollBarPosition, &QActionGroup::triggered,
            m_tabWidget, &TabWidget::changeScrollPosition);

    QMenu *scrollBarPositionMenu = new QMenu(tr("Scrollbar Layout"), menu);
    scrollBarPositionMenu->addActions(scrollBarPosition->actions());

    menu->addMenu(scrollBarPositionMenu);

    menuBar()->addMenu(menu);
}

void MainWindow::setupHelpMenu()
{
    QMenu *menu = new QMenu(tr("&Help"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::About);
    connect(action, &QAction::triggered, this, &MainWindow::showAboutMessageBox);
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::AboutQt);
    connect(action, &QAction::triggered, qApp, &QApplication::aboutQt);
    addAction(action);
    menu->addAction(action);

    menuBar()->addMenu(menu);
}

void MainWindow::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    m_contextMenu->addAction(m_actionManager->action(ActionId::Copy));
    m_contextMenu->addAction(m_actionManager->action(ActionId::Paste));
    m_contextMenu->addAction(m_actionManager->action(ActionId::PasteSelection));
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionManager->action(ActionId::Clear));
    m_contextMenu->addSeparator();

    QMenu *zoomMenu = new QMenu(tr("&Zoom"), m_contextMenu);

    /// TODO: Move to View Menu
    QAction *action;
    action = m_actionManager->action(ActionId::ZoomIn);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->zoomIn();
    });
    addAction(action);
    zoomMenu->addAction(action);

    action = m_actionManager->action(ActionId::ZoomOut);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->zoomOut();
    });
    addAction(action);
    zoomMenu->addAction(action);

    zoomMenu->addSeparator();

    action = m_actionManager->action(ActionId::ZoomReset);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->zoomReset();
    });
    addAction(action);
    zoomMenu->addAction(action);

    m_contextMenu->addMenu(zoomMenu);

    m_contextMenu->addSeparator();

    action = m_actionManager->action(ActionId::SplitHorizontally);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::splitHorizontally);
    addAction(action);
    m_contextMenu->addAction(action);

    action = m_actionManager->action(ActionId::SplitVertically);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::splitVertically);
    addAction(action);
    m_contextMenu->addAction(action);

    m_contextMenu->addSeparator();

    action = m_actionManager->action(ActionId::CloseTerminal);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::splitCollapse);
    addAction(action);
    m_contextMenu->addAction(action);

    m_tabWidget->setContextMenu(m_contextMenu);
}

void MainWindow::setupWindowActions()
{
    QAction *action;
    action = m_actionManager->action(ActionId::NextTab);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::switchToRight);
    addAction(action);

    action = m_actionManager->action(ActionId::PreviousTab);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::switchToLeft);
    addAction(action);
}

void MainWindow::toggleTabBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowTabs)->isChecked();
    m_tabWidget->tabBar()->setVisible(newVisible);
    m_preferences->hideTabBar = !newVisible;
}

void MainWindow::toggleMenuBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowMenu)->isChecked();
    menuBar()->setVisible(newVisible);
    m_preferences->menuVisible = newVisible;
}

void MainWindow::showAboutMessageBox()
{
    QMessageBox::about(this, QString("About %1").arg(qApp->applicationName()),
                       tr("A lightweight terminal emulator"));
}

void MainWindow::showPreferencesDialog()
{
    QScopedPointer<PreferencesDialog> pd(new PreferencesDialog(this));
    pd->exec();
}

void MainWindow::preferencesChanged()
{
    QApplication::setStyle(m_preferences->guiStyle);
    m_tabWidget->setTabPosition((QTabWidget::TabPosition)m_preferences->tabBarPosition);
    m_tabWidget->preferencesChanged();

    menuBar()->setVisible(m_preferences->menuVisible);

    m_preferences->save();
    realign();
}

void MainWindow::realign()
{
    if (!m_dropDownMode)
        return;
    QRect desktop = QApplication::desktop()->availableGeometry(this);
    QRect geometry = QRect(0, 0,
                           desktop.width()  * m_preferences->dropWidht  / 100,
                           desktop.height() * m_preferences->dropHeight / 100);
    geometry.moveCenter(desktop.center());
    // do not use 0 here - we need to calculate with potential panel on top
    geometry.setTop(desktop.top());
    setGeometry(geometry);
}

void MainWindow::showHide()
{
    if (isVisible()) {
        hide();
    } else {
        realign();
        show();
        activateWindow();
    }
}

void MainWindow::setKeepOpen(bool value)
{
    m_preferences->dropKeepOpen = value;
    if (!m_dropDownLockButton)
        return;

    if (value)
        m_dropDownLockButton->setIcon(QIcon(QStringLiteral(":/icons/locked.png")));
    else
        m_dropDownLockButton->setIcon(QIcon(QStringLiteral(":/icons/notlocked.png")));

    m_dropDownLockButton->setChecked(value);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_preferences->askOnExit || !m_tabWidget->count()) {
        m_preferences->mainWindowGeometry = saveGeometry();
        m_preferences->mainWindowState = saveState();
        event->accept();
        return;
    }

    QScopedPointer<QMessageBox> mb(new QMessageBox(this));
    mb->setWindowTitle(tr("Exit %1").arg(qApp->applicationName()));
    mb->setText(tr("Are you sure you want to exit?"));
    mb->setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    QCheckBox *dontAskCheckBox = new QCheckBox(tr("Do not ask again"), mb.data());
    mb->setCheckBox(dontAskCheckBox);

    if (mb->exec() == QMessageBox::Yes) {
        m_preferences->mainWindowGeometry = saveGeometry();
        m_preferences->mainWindowState = saveState();
        m_preferences->askOnExit = !dontAskCheckBox->isChecked();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate
            && m_dropDownMode
            && !m_preferences->dropKeepOpen
            && qApp->activeWindow() == nullptr) {
        hide();
    }
    return QMainWindow::event(event);
}

TerminalWidget *MainWindow::currentTerminal() const
{
    return m_tabWidget->terminalHolder()->currentTerminal();
}
