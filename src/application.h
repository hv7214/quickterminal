#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class QxtGlobalShortcut;

class MainWindow;
class Preferences;

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

signals:

public slots:
    void createWindow();
    void quit();

private slots:
    void preferencesChanged();
    void windowDeleted(QObject *object);

private:
    void parseOptions();
    void setupActions();
    void setDropShortcut(const QKeySequence &shortcut);
    void loadUserShortcuts();

    Preferences * const m_preferences = nullptr;
    QList<MainWindow *> m_windows;

    // Command line options
    QString m_command;
    bool m_dropMode = false;
    QString m_workingDir;

    QxtGlobalShortcut *m_dropShortcut = nullptr;
};

#endif // APPLICATION_H
