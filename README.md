# 🔐 SSH Key Manager

一个使用 **C++ / Qt** 开发的图形化 SSH 密钥管理工具，帮助开发者更方便地管理本地 `.ssh` 密钥文件，支持添加、删除、设置默认密钥，以及复制公钥等常用操作。

---

## ✨ 功能特性

- 🚀 自动扫描 `~/.ssh` 目录下的所有密钥
- ➕ 一键添加新密钥（可生成 SSH 密钥对）
- 🗑️ 删除指定的密钥文件
- ✅ 设置默认密钥（自动替换 `~/.ssh/id_rsa` 文件）
- 📋 显示公钥内容，可复制到剪贴板
- 🔄 刷新密钥列表
- 🖼️ 图形化界面，跨平台支持

---

## 🖥️ 界面预览
![图片](https://github.com/user-attachments/assets/e21d09c2-9579-4d96-a1f3-6f5a1e44d4bd)

---

## 🛠️ 编译与运行

### 依赖项

- Qt 5.x 或 6.x（建议使用 Qt Creator 构建）
- OpenSSH（需本地可用 `ssh-keygen` 命令）

### 编译步骤

```bash
git clone https://github.com/wenhaohao0818/SSHKeyManager/.git
cd SSHKeyManager
# 打开项目文件（.pro 或 CMakeLists.txt）使用 Qt Creator 构建并运行
```

---

## 📁 项目结构

```bash
.
├── Demo
│   ├── Demo.rar
├── logo.ico
├── logo.rc
├── main.cpp
├── mainwindow.cpp
├── mainwindow.h
├── mainwindow.ui
├── res.qrc
├── SSHKeyManager.pro
├── static
│   ├── logo.png
└── README.md             # 当前文档
```

---

## 📋 LICENSE

本项目使用 [MIT License](LICENSE)。

> 你可以自由使用、修改和分发本项目，**前提是需保留原作者署名信息**。

---

## 🙋‍♂️ 作者

凉风有信（LiangfengYouxin）  
GitHub: [https://github.com/wenhaohao0818](https://github.com/wenhaohao0818)
Blog: [https://blog.wenhaohao.xyz](https://blog.wenhaohao.xyz)

---

## ⭐️ 欢迎贡献与支持

如果你觉得这个工具对你有帮助，欢迎 **Star ⭐**、**Fork 🍴** 或提 Issue！  
你的支持是我继续优化的动力 💪
