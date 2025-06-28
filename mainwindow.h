#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardPaths>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QListWidget>
#include <QClipboard>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProcess>
#include <QDialogButtonBox>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;

    QString mSSHWorkDir;
    QString mSSHKeygenPath;
    QString GetDefaultKey();
    QString GetPubKeyTitle(QString pub_key_path);
    void RefreshUi();
    QString sanitizeFolderName(const QString &input);
    void LoadKeysList();
    void InitConfig();
    void SetTextView(QString text);
    QString GetTextView();
    void SelectKeyItemByTitle(const QString &targetTitle);
    void showAboutDialog();
    QString QueryGitSshKeygenPath();
public slots:
    void Slot_Add_key();
    void Slot_Del_key();
    void Slot_RefreshList_key();
    void Slot_SetDefaultKey();
    void Slot_GenKey();
    void Slot_CopyPubKey();
    void Slot_onKeyItemClicked(QListWidgetItem *item);

};
#endif // MAINWINDOW_H
