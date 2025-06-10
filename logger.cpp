#include "logger.h"
#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

void Logger::info(const QString &message) {
  QFile file("bot.log");
  if (file.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
           << " - INFO: " << message << "\n";
    file.close();
  }
}

void Logger::error(const QString &message) {
  QFile file("bot.log");
  if (file.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
           << " - ERROR: " << message << "\n";
    file.close();
  }
  QMessageBox::critical(nullptr, "Ошибка", message);
}
