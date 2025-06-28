#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QIcon appIcon(":/static/logo.png");  // 从资源文件加载
    setWindowIcon(appIcon);
    setWindowTitle("SSH Key管理器");

    InitConfig();
    LoadKeysList();
    RefreshUi();

    connect(ui->bt_add_key,&QPushButton::clicked,this,&MainWindow::Slot_Add_key);
    connect(ui->bt_del_key,&QPushButton::clicked,this,&MainWindow::Slot_Del_key);
    connect(ui->bt_gen_key,&QPushButton::clicked,this,&MainWindow::Slot_GenKey);
    connect(ui->bt_refrsh_list,&QPushButton::clicked,this,&MainWindow::Slot_RefreshList_key);
    connect(ui->bt_copy_pub_key,&QPushButton::clicked,this,&MainWindow::Slot_CopyPubKey);
    connect(ui->bt_set_default_key,&QPushButton::clicked,this,&MainWindow::Slot_SetDefaultKey);
    connect(ui->lv_key_list, &QListWidget::itemClicked,this, &MainWindow::Slot_onKeyItemClicked);
    connect(ui->about, &QMenu::aboutToShow, this, &MainWindow::showAboutDialog);

}

void MainWindow::RefreshUi()
{
    ui->lb_work_path_show->setText("目录:"+mSSHWorkDir);
    QString defaultTitle = GetDefaultKey();
    if(defaultTitle.isEmpty()){
        ui->lb_default_key_name->setText("当前密钥:不存在");
    }else{
        ui->lb_default_key_name->setText("当前密钥:"+defaultTitle);
        SelectKeyItemByTitle(defaultTitle);
    }
}

QString MainWindow::GetDefaultKey()
{
    if(mSSHWorkDir.isEmpty())
    {
        return "";
    }
    QString pub_key_path = mSSHWorkDir + "/id_rsa.pub";
    QString pri_key_path = mSSHWorkDir + "/id_rsa";
    return GetPubKeyTitle(pub_key_path);
}

QString MainWindow::GetPubKeyTitle(QString pub_key_path)
{
    QFile file(pub_key_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << pub_key_path;
        return QString();
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString line = in.readLine().trimmed();
    qDebug()<<line<<endl;
    // SSH 公钥格式通常为：ssh-rsa <base64_key> <comment>
    QStringList parts = line.split(' ');
    if (parts.size() >= 3) {
        return parts[2];  // 第三个字段是标题或注释
    } else {
        qWarning() << "公钥格式不正确:" << line;
        return QString();
    }
}
QString MainWindow::sanitizeFolderName(const QString& input) {
    // Windows 不允许的字符：< > : " / \ | ? *
    QRegExp invalidChars(R"([ <>:"/\\|?*])");
    QString result = input;  // 复制一份
    result.remove(invalidChars);
    return result;
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::LoadKeysList()
{
    ui->lv_key_list->clear(); // 清空列表

    QString keysPath = mSSHWorkDir + "/keys";
    QDir keysDir(keysPath);

    if (!keysDir.exists()) {
        qWarning() << "keys 目录不存在：" << keysPath;
        return;
    }

    // 遍历 keys 目录的所有子目录
    QFileInfoList subDirs = keysDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &subDirInfo : subDirs) {
        QString pubKeyPath = subDirInfo.absoluteFilePath() + "/id_rsa.pub";

        if (QFile::exists(pubKeyPath)) {
            QString title = GetPubKeyTitle(pubKeyPath);
            qDebug()<<"select name:"<<title<<endl;
            if (!title.isEmpty()) {
                ui->lv_key_list->addItem(title);
            }
        }
    }
}
void MainWindow::InitConfig()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QFile configFile(configPath);

    // 如果文件不存在，创建并写入默认配置
    if (!configFile.exists()) {
        QSettings settings(configPath, QSettings::IniFormat);
        settings.beginGroup("General");
        settings.setValue("work_path", QDir::homePath()+"/.ssh");

        // 查询 git 安装路径并定位 ssh-keygen.exe
        QString sshKeygenPath = QueryGitSshKeygenPath();
        settings.setValue("ssh_keygen_path", sshKeygenPath);
        settings.endGroup();
        qDebug() << "配置文件不存在，已创建默认配置: " << configPath;
    }
    // 读取配置
    QSettings settings(configPath, QSettings::IniFormat);
    mSSHWorkDir = settings.value("General/work_path").toString();
    mSSHKeygenPath = settings.value("General/ssh_keygen_path").toString();
    if(!mSSHWorkDir.isEmpty())
    {
        //  检查 ~/.ssh 目录是否存在，不存在则创建
        QDir dir;
        if (!dir.exists(mSSHWorkDir)) {
            if (dir.mkpath(mSSHWorkDir)) {
                qDebug() << ".ssh 目录不存在，已创建: " << mSSHWorkDir;
            } else {
                qWarning() << "无法创建 .ssh 目录: " << mSSHWorkDir;
            }
        } else {
            qDebug() << ".ssh 目录已存在: " << mSSHWorkDir;
        }
        dir.mkpath(mSSHWorkDir + "/keys");
    }

    QString defaultKeyName = GetDefaultKey();
    if(!defaultKeyName.isEmpty())
    {
        QDir dir;
        defaultKeyName = sanitizeFolderName(defaultKeyName);
        QString keysDir = mSSHWorkDir + "/keys/" + defaultKeyName;
        if(dir.mkpath(keysDir))
        {
            // 拷贝文件
            QString pubKeySrc = mSSHWorkDir + "/id_rsa.pub";
            QString priKeySrc = mSSHWorkDir + "/id_rsa";
            QString pubKeyDst = keysDir + "/id_rsa.pub";
            QString priKeyDst = keysDir + "/id_rsa";
            bool successPub = QFile::copy(pubKeySrc, pubKeyDst);
            bool successPri = QFile::copy(priKeySrc, priKeyDst);
        }
    }
}

void MainWindow::SetTextView(QString text)
{
    ui->tv_pub_key_show->clear();
    ui->tv_pub_key_show->setText(text);
}

QString MainWindow::GetTextView()
{

}

void MainWindow::Slot_Add_key()
{
    qDebug()<<"click add"<<endl;
    // 添加密钥 打开文件选择对话框 支持选择私钥或公钥文件 获取到文件所在目录的路径后，判断私钥和公钥是否存在，若存在则提取注释名 将私钥和公钥复制到 keys/注释名目录下
    // 选择私钥或公钥文件
    QString filePath = QFileDialog::getOpenFileName(this,
        "选择 SSH 密钥文件", QDir::homePath() + "/.ssh", "SSH Key (id_rsa or id_rsa.pub)");

    if (filePath.isEmpty()) {
        return; // 用户取消
    }

    // 获取文件所在目录
    QFileInfo fileInfo(filePath);
    QString keyDirPath = fileInfo.absolutePath();
    QString pubKeyPath = keyDirPath + "/id_rsa.pub";
    QString priKeyPath = keyDirPath + "/id_rsa";

    // 检查是否同时存在公钥和私钥
    if (!QFile::exists(pubKeyPath) || !QFile::exists(priKeyPath)) {
        QMessageBox::warning(this, "错误", "公钥或私钥文件缺失，请确保目录中包含 id_rsa 和 id_rsa.pub");
        return;
    }

    // 提取注释名（公钥第三段）
    QString title;
    {
        QFile pubFile(pubKeyPath);
        if (pubFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&pubFile);
            QString line = in.readLine().trimmed();
            QStringList parts = line.split(' ');
            if (parts.size() >= 3) {
                title = parts[2];
            }
        }
    }

    if (title.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法从公钥中提取注释名，无法继续添加。");
        return;
    }

    // 构造目标路径 ~/.ssh/keys/<title>/
    QString destDirPath = mSSHWorkDir + "/keys/" + sanitizeFolderName(title);
    QDir dir;
    if (!dir.exists(destDirPath)) {
        if (!dir.mkpath(destDirPath)) {
            QMessageBox::warning(this, "错误", "无法创建密钥目录：" + destDirPath);
            return;
        }
    }

    // 复制密钥
    QString destPub = destDirPath + "/id_rsa.pub";
    QString destPri = destDirPath + "/id_rsa";

    QFile::remove(destPub);
    QFile::remove(destPri);

    bool pubCopied = QFile::copy(pubKeyPath, destPub);
    bool priCopied = QFile::copy(priKeyPath, destPri);

    if (pubCopied && priCopied) {
        QMessageBox::information(this, "成功", "密钥已成功添加。");
        Slot_RefreshList_key(); // 刷新密钥列表
    } else {
        QMessageBox::warning(this, "错误", "复制密钥失败，请检查权限或文件是否被占用。");
    }
}

void MainWindow::Slot_Del_key()
{
    // 获取ui->lv_key_list中选中的key 进行删除 若是当前默认key则不允许删除 若不是 则弹出提示框提示是否删除
    QListWidgetItem *currentItem = ui->lv_key_list->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个密钥！");
        return;
    }

    QString selectedKeyName = currentItem->text(); // 假设格式是 "keyName - title"
    QString defaultKeyName = GetDefaultKey();
    if (selectedKeyName == defaultKeyName) {
        QMessageBox::warning(this, "禁止删除", "当前默认密钥不能删除！");
        return;
    }

    // 弹出确认对话框
    auto ret = QMessageBox::question(this, "确认删除",
                                     QString("确定删除密钥 \"%1\" 吗？").arg(selectedKeyName),
                                     QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // 执行删除逻辑，比如删除 keys/selectedKeyName 目录及内容
        QString dirToRemove = mSSHWorkDir + "/keys/" + sanitizeFolderName(selectedKeyName);
        qDebug()<<dirToRemove<<endl;
        QDir dir(dirToRemove);
        if (dir.exists()) {
            bool removed = dir.removeRecursively();
            if (removed) {
                // 从列表中移除项
                delete currentItem;
                //QMessageBox::information(this, "删除成功", "密钥已删除");
            } else {
                QMessageBox::warning(this, "删除失败", "无法删除密钥文件夹");
            }
        } else {
            QMessageBox::warning(this, "错误", "密钥文件夹不存在");
        }
    }
}

void MainWindow::Slot_RefreshList_key()
{
    LoadKeysList();
    RefreshUi();
}

void MainWindow::Slot_SetDefaultKey()
{
    // 获取当前密钥title 获取选中密钥title 若相同则不做处理 若不相同则提示是否设置默认密钥 若选择是 则将 keys下对应目录中的文件复制到.ssh中覆盖公钥和私钥
    QListWidgetItem *currentItem = ui->lv_key_list->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个密钥！");
        return;
    }

    // 获取选中密钥的title，假设格式："keyName - title"
    QString selectedText = currentItem->text();
    QString selectedTitle = selectedText;
    QString defaultKeyTitle = GetDefaultKey();
    if (selectedTitle == defaultKeyTitle) {
        // 和当前默认密钥相同，不处理
        QMessageBox::information(this, "提示", "已是默认密钥，无需设置。");
        return;
    }

    // 确认是否设置默认密钥
    auto ret = QMessageBox::question(this, "设置默认密钥",
                                     QString("确定将 \"%1\" 设置为默认密钥吗？").arg(selectedTitle),
                                     QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // 目标路径（~/.ssh）
        QString sshDir = mSSHWorkDir;  // 例如 "C:/Users/WenHao/.ssh"

        // 选中密钥目录
        QString selectedKeyName = sanitizeFolderName(selectedText);
        QString sourceDir = sshDir + "/keys/" + selectedKeyName;

        QString srcPriKey = sourceDir + "/id_rsa";
        QString srcPubKey = sourceDir + "/id_rsa.pub";

        QString dstPriKey = sshDir + "/id_rsa";
        QString dstPubKey = sshDir + "/id_rsa.pub";

        // 先检查源文件是否存在
        if (!QFile::exists(srcPriKey) || !QFile::exists(srcPubKey)) {
            QMessageBox::warning(this, "错误", "选中密钥文件不存在，无法设置默认密钥。");
            return;
        }

        // 复制覆盖私钥和公钥，先删除旧文件（防止copy失败）
        QFile::remove(dstPriKey);
        QFile::remove(dstPubKey);

        bool priOk = QFile::copy(srcPriKey, dstPriKey);
        bool pubOk = QFile::copy(srcPubKey, dstPubKey);

        if (priOk && pubOk) {
            QMessageBox::information(this, "成功", "默认密钥已设置。");
        } else {
            QMessageBox::warning(this, "失败", "设置默认密钥失败，请检查文件权限。");
        }
    }
    RefreshUi();
}

void MainWindow::Slot_GenKey()
{
    // 生成密钥 调用ssk-keygen生成 git 密钥 要求输入注释名 且不能输入非法字符（不能创建文件夹的字符 空格也不能包含）
    bool ok = false;
    QString title = QInputDialog::getText(this, "输入注释名", "请输入密钥注释名（不能包含非法字符）", QLineEdit::Normal, "", &ok);

    if (!ok || title.trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入无效", "密钥注释名不能为空！");
        return;
    }

    // 检查非法字符（空格和 Windows 文件名非法字符）
    QRegularExpression invalidChars(R"([ <>:"/\\|?*])");
    if (title.contains(invalidChars)) {
        QMessageBox::warning(this, "无效注释", "注释名包含非法字符，不能用于目录名！");
        return;
    }

    QString cleanTitle = sanitizeFolderName(title).trimmed();
    QString keyDir = mSSHWorkDir + "/keys/" + cleanTitle;

    QDir dir;
    if (!dir.exists(keyDir)) {
        if (!dir.mkpath(keyDir)) {
            QMessageBox::warning(this, "创建目录失败", "无法创建目录：" + keyDir);
            return;
        }
    }

    QString keyPath = keyDir + "/id_rsa";

    // 检查是否已存在
    if (QFile::exists(keyPath) || QFile::exists(keyPath + ".pub")) {
        QMessageBox::StandardButton ret = QMessageBox::question(this, "覆盖密钥？",
            "该注释对应的密钥已存在，是否覆盖？",
            QMessageBox::Yes | QMessageBox::No);

        if (ret != QMessageBox::Yes)
            return;

        QFile::remove(keyPath);
        QFile::remove(keyPath + ".pub");
    }
    if (!QFile::exists(mSSHKeygenPath)) {
        QMessageBox::critical(this, "错误", "未找到 ssh-keygen.exe，路径：" + mSSHKeygenPath+"请手动在配置文件中设置");
        return;
    }
    // 调用 ssh-keygen 生成密钥
    QProcess process;
    QStringList args;
    args << "-t" << "rsa"
//         << "-b" << "2048"
         << "-f" << keyPath
         << "-C" << cleanTitle
         << "-N" << ""; // 空密码
    process.start(mSSHKeygenPath, args);
    if (!process.waitForFinished()) {
        QMessageBox::critical(this, "失败", "ssh-keygen 执行失败，请确认已安装。");
        return;
    }

    QMessageBox::information(this, "成功", "密钥生成成功！");
    Slot_RefreshList_key();

}

void MainWindow::Slot_CopyPubKey()
{
    QString text = ui->tv_pub_key_show->toPlainText();
    // 获取系统剪贴板
    QClipboard *clipboard = QGuiApplication::clipboard();
    // 设置纯文本到剪贴板
    clipboard->setText(text);
}

void MainWindow::Slot_onKeyItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    QString clickedText = item->text();
    QString clicked_key_path = mSSHWorkDir + "/keys/" + sanitizeFolderName(clickedText);
    qDebug()<<clicked_key_path<<endl;
    QString clicked_pub_key_path = clicked_key_path + "/id_rsa.pub";
    QString clicked_pri_key_path = clicked_key_path + "/id_rsa";
    QFile file(clicked_pub_key_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qWarning() << "无法打开文件:" << clicked_pub_key_path;
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString line = in.readLine().trimmed();
    SetTextView(line);
}

void MainWindow::SelectKeyItemByTitle(const QString& targetTitle)
{
    for (int i = 0; i < ui->lv_key_list->count(); ++i) {
        QListWidgetItem* item = ui->lv_key_list->item(i);
        if (!item) continue;

        QString itemTitle = item->text();
        if (itemTitle == targetTitle) {
            // 选中该项
            item->setSelected(true);
            // 也可滚动到该项并设为当前项
            ui->lv_key_list->setCurrentItem(item);
            ui->lv_key_list->scrollToItem(item);
            Slot_onKeyItemClicked(item);
            break;  // 若只选中一个，找到就可以退出
        }
    }
}

void MainWindow::showAboutDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("关于 SSH 密钥管理器");
    dialog.setFixedSize(400, 250);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel(&dialog);
    label->setTextFormat(Qt::RichText);
    label->setOpenExternalLinks(true);  // 允许点击链接
    label->setText(R"(
        <h2>SSH 密钥管理器</h2>
        <p>版本：1.0.0</p>
        <p>作者：liangfengyouxin</p>
        <p>功能：用于管理、添加、删除和切换 SSH 密钥</p>
        <p>GitHub：<a href='https://blog.wenhaohao.xyz'>https://blog.wenhaohao.xyz</a></p>
        <p>© 2025 All rights reserved.</p>
    )");
    label->setAlignment(Qt::AlignCenter);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(buttonBox);

    dialog.exec();  // 显示模态对话框
}

// 查询 Git 安装路径，返回 ssh-keygen.exe 的完整路径，找不到返回空字符串
QString MainWindow::QueryGitSshKeygenPath()
{
    QProcess regQuery;
    regQuery.start("reg", QStringList() << "query" << R"(HKEY_LOCAL_MACHINE\SOFTWARE\GitForWindows)" << "/v" << "InstallPath");
    if (!regQuery.waitForFinished(3000)) {
        qWarning() << "查询注册表超时";
        return QString();
    }
    QString regOutput = regQuery.readAllStandardOutput();
    QRegularExpression regExp(R"(InstallPath\s+REG_SZ\s+(.+))");
    QRegularExpressionMatch match = regExp.match(regOutput);
    if (!match.hasMatch()) {
        qWarning() << "未解析到 Git 安装路径";
        return QString();
    }

    QString gitPath = match.captured(1).trimmed();

    // 常见 ssh-keygen.exe 路径
    QStringList candidates = {
        gitPath + "/usr/bin/ssh-keygen.exe",
        gitPath + "/bin/ssh-keygen.exe"
    };

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            qDebug() << "找到 ssh-keygen.exe 路径：" << path;
            return path;
        }
    }

    qWarning() << "未找到 ssh-keygen.exe 在 Git 安装目录下";
    return QString();
}

