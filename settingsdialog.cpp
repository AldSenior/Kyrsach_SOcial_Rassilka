#include "settingsdialog.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>
#include <QLabel>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Настройки");
    setMinimumSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    targetList = new QListWidget;
    mainLayout->addWidget(new QLabel("Цели публикации:"));
    mainLayout->addWidget(targetList);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addBtn = new QPushButton("Добавить");
    QPushButton *editBtn = new QPushButton("Редактировать");
    QPushButton *removeBtn = new QPushButton("Удалить");
    QPushButton *exportBtn = new QPushButton("Экспорт настроек");
    QPushButton *importBtn = new QPushButton("Импорт настроек");
    QObject::connect(addBtn, &QPushButton::clicked, this, &SettingsDialog::addTarget);
    QObject::connect(editBtn, &QPushButton::clicked, this, &SettingsDialog::editTarget);
    QObject::connect(removeBtn, &QPushButton::clicked, this, &SettingsDialog::removeTarget);
    QObject::connect(exportBtn, &QPushButton::clicked, this, &SettingsDialog::exportSettings);
    QObject::connect(importBtn, &QPushButton::clicked, this, &SettingsDialog::importSettings);
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(importBtn);
    mainLayout->addLayout(buttonLayout);

    QSettings settings;
    QJsonDocument doc = QJsonDocument::fromJson(settings.value("post_targets").toByteArray());
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();
        targetList->addItem(QString("%1: %2").arg(obj["platform"].toString(), obj["target_id"].toString()));
    }
}

void SettingsDialog::addTarget() {
    TargetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString platform = dialog.getPlatform();
        QString token = dialog.getToken();
        QString targetId = dialog.getTargetId();
        if (platform.isEmpty() || token.isEmpty() || targetId.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены!");
            return;
        }

        QSettings settings;
        QJsonDocument doc = QJsonDocument::fromJson(settings.value("post_targets").toByteArray());
        QJsonArray targets = doc.array();
        QJsonObject obj;
        obj["id"] = QUuid::createUuid().toString();
        obj["platform"] = platform;
        obj["token"] = token;
        obj["target_id"] = targetId;
        targets.append(obj);
        settings.setValue("post_targets", QJsonDocument(targets).toJson());

        targetList->addItem(QString("%1: %2").arg(platform, targetId));
    }
}

void SettingsDialog::editTarget() {
    QListWidgetItem *item = targetList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите цель для редактирования!");
        return;
    }
    // Логика редактирования (в разработке)
    QMessageBox::information(this, "Редактирование", "Функция редактирования в разработке!");
}

void SettingsDialog::removeTarget() {
    QListWidgetItem *item = targetList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите цель для удаления!");
        return;
    }
    QSettings settings;
    QJsonDocument doc = QJsonDocument::fromJson(settings.value("post_targets").toByteArray());
    QJsonArray targets = doc.array();
    QJsonArray newTargets;
    QString targetStr = item->text();
    for (const auto &val : targets) {
        QJsonObject obj = val.toObject();
        if (QString("%1: %2").arg(obj["platform"].toString(), obj["target_id"].toString()) != targetStr) {
            newTargets.append(obj);
        }
    }
    settings.setValue("post_targets", QJsonDocument(newTargets).toJson());
    delete item;
}

void SettingsDialog::exportSettings() {
    QString filePath = QFileDialog::getSaveFileName(this, "Экспорт настроек", "", "JSON (*.json)");
    if (filePath.isEmpty())
        return;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QSettings settings;
        file.write(settings.value("post_targets").toByteArray());
        file.close();
        QMessageBox::information(this, "Успех", "Настройки экспортированы!");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл!");
    }
}

void SettingsDialog::importSettings() {
    QString filePath = QFileDialog::getOpenFileName(this, "Импорт настроек", "", "JSON (*.json)");
    if (filePath.isEmpty())
        return;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QSettings settings;
        settings.setValue("post_targets", file.readAll());
        file.close();
        targetList->clear();
        QJsonDocument doc = QJsonDocument::fromJson(settings.value("post_targets").toByteArray());
        for (const auto &val : doc.array()) {
            QJsonObject obj = val.toObject();
            targetList->addItem(QString("%1: %2").arg(obj["platform"].toString(), obj["target_id"].toString()));
        }
        QMessageBox::information(this, "Успех", "Настройки импортированы!");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл!");
    }
}
