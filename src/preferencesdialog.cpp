#include "preferencesdialog.h"

#include "actionmanager.h"
#include "preferences.h"

#include <qtermwidget.h>

#include <QFileDialog>
#include <QFontDialog>
#include <QStyleFactory>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(changeFontButton, SIGNAL(clicked()),
            this, SLOT(changeFontButton_clicked()));

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();

    const Preferences * const preferences = Preferences::instance();

    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(preferences->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(preferences->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0);

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << "No scrollbar" << "Left" << "Right";
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(preferences->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << "Top" << "Bottom" << "Left" << "Right";
    tabsPos_comboBox->addItems(tabsPosList);
    tabsPos_comboBox->setCurrentIndex(preferences->tabsPos);

    alwaysShowTabsCheckBox->setChecked(preferences->alwaysShowTabs);

    // show main menu bar
    showMenuCheckBox->setChecked(preferences->menuVisible);

    /* actions by motion after paste */

    QStringList motionAfter;
    motionAfter << "No move" << "Move start" << "Move end";
    motionAfterPasting_comboBox->addItems(motionAfter);
    motionAfterPasting_comboBox->setCurrentIndex(preferences->m_motionAfterPaste);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(preferences->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);

    setFontSample(preferences->font);

    appOpacityBox->setValue(preferences->appOpacity);
    // connect(appOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    termOpacityBox->setValue(preferences->termOpacity);
    // connect(termOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    highlightCurrentCheckBox->setChecked(preferences->highlightCurrentTerminal);

    askOnExitCheckBox->setChecked(preferences->askOnExit);

    useCwdCheckBox->setChecked(preferences->useCWD);

    historyLimited->setChecked(preferences->historyLimited);
    historyUnlimited->setChecked(!preferences->historyLimited);
    historyLimitedTo->setValue(preferences->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(preferences->dropShowOnStart);
    dropHeightSpinBox->setValue(preferences->dropHeight);
    dropWidthSpinBox->setValue(preferences->dropWidht);
    dropShortCutEdit->setText(preferences->dropShortCut.toString());


    actionTreeWidget->sortByColumn(0, Qt::AscendingOrder);

    QFont userEditedShortcutFont;
    userEditedShortcutFont.setBold(true);
    userEditedShortcutFont.setItalic(true);

    // Populate shortcuts
    foreach (const ActionInfo &actionInfo, ActionManager::registry()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(actionTreeWidget);
        item->setText(0, ActionManager::clearActionText(actionInfo.text));
        if (actionInfo.shortcut.isEmpty()) {
            item->setText(1, actionInfo.defaultShortcut.toString());
        } else {
            item->setFont(1, userEditedShortcutFont);
            item->setText(1, actionInfo.shortcut.toString());
        }
    }
}

void PreferencesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PreferencesDialog::apply()
{
    Preferences * const preferences = Preferences::instance();
    preferences->colorScheme = colorSchemaCombo->currentText();
    preferences->font = fontSampleLabel->font(); // fontComboBox->currentFont();
    preferences->guiStyle = (styleComboBox->currentText() == tr("System Default"))
            ? QString() : styleComboBox->currentText();

    preferences->emulation = emulationComboBox->currentText();

    /* do not allow to go above 99 or we lose transparency option */
    preferences->appOpacity = qMin(appOpacityBox->value(), 99);

    preferences->termOpacity = termOpacityBox->value();
    preferences->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();

    preferences->askOnExit = askOnExitCheckBox->isChecked();

    preferences->useCWD = useCwdCheckBox->isChecked();

    preferences->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    preferences->tabsPos = tabsPos_comboBox->currentIndex();
    preferences->alwaysShowTabs = alwaysShowTabsCheckBox->isChecked();
    preferences->menuVisible = showMenuCheckBox->isChecked();
    preferences->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();

    preferences->historyLimited = historyLimited->isChecked();
    preferences->historyLimitedTo = historyLimitedTo->value();

    saveShortcuts();

    preferences->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    preferences->dropHeight = dropHeightSpinBox->value();
    preferences->dropWidht = dropWidthSpinBox->value();
    preferences->dropShortCut = QKeySequence(dropShortCutEdit->text());

    preferences->save();

    preferences->emitChanged();
}

void PreferencesDialog::setFontSample(const QFont &f)
{
    fontSampleLabel->setFont(f);
    QString sample("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}

void PreferencesDialog::changeFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fontSampleLabel->font(), this,
                                      tr("Select Terminal Font"), QFontDialog::MonospacedFonts);
    if (!ok)
        return;
    setFontSample(font);
}

void PreferencesDialog::saveShortcuts()
{
    /// TODO: ActionManager support
}
