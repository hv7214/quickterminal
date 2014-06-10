#ifndef TERMWIDGETHOLDER_H
#define TERMWIDGETHOLDER_H

#include "termwidget.h"

#include <QWidget>

class QSplitter;

/*! \brief TermWidget group/session manager.

This widget (one per TabWidget tab) is a "proxy" widget beetween TabWidget and
unspecified count of TermWidgets. Basically it should look like a single TermWidget
for TabWidget - with its signals and slots.

Splitting and collapsing of TermWidgets is done here.
*/
class TermWidgetHolder : public QWidget
{
    Q_OBJECT

public:
    explicit TermWidgetHolder(const QString &wdir, const QString &shell = QString(),
                              QWidget *parent = nullptr);

    void propertiesChanged();
    void setInitialFocus();

    void loadSession();
    void saveSession(const QString &name);
    void zoomIn(uint step);
    void zoomOut(uint step);

    TermWidget *currentTerminal() const;

public slots:
    void splitHorizontal(TermWidget *term);
    void splitVertical(TermWidget *term);
    void splitCollapse(TermWidget *term);
    void setWDir(const QString &wdir);
    void switchNextSubterminal();
    void switchPrevSubterminal();

signals:
    void terminalContextMenuRequested(const QPoint &pos);
    void finished();
    void lastTerminalClosed();
    void renameSession();

private:
    QString m_wdir;
    QString m_shell;
    TermWidget *m_currentTerm = nullptr;

    void split(TermWidget *term, Qt::Orientation orientation);
    TermWidget *newTerm(const QString &wdir = QString(), const QString &shell = QString());

private slots:
    void setCurrentTerminal(TermWidget *term);
    void handle_finished();
};

#endif // TERMWIDGETHOLDER_H
