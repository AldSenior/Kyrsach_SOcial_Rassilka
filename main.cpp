#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  app.setStyleSheet(R"(
        QMainWindow { background-color: #2b2b2b; color: #ffffff; }
        QMenuBar { background-color: #3c3f41; color: #ffffff; }
        QMenuBar::item:selected { background-color: #505050; }
        QToolBar { background-color: #3c3f41; border: none; }
        QToolButton:hover { background-color: #505050; }
        QStatusBar { background-color: #3c3f41; color: #ffffff; }
        QPushButton { background-color: #007bff; color: #ffffff; border: none; padding: 10px; border-radius: 4px; }
        QPushButton:hover { background-color: #0056b3; }
        QTextEdit, QLineEdit { background-color: #ffffff; color: #333333; border: 1px solid #ced4da; border-radius: 4px; padding: 5px; }
        QComboBox, QListWidget { background-color: #ffffff; color: #333333; border: 1px solid #ced4da; border-radius: 4px; }
        QGroupBox { border: 1px solid #4a4a4a; border-radius: 4px; margin-top: 10px; padding: 10px; }
        QLabel { font-size: 14px; margin-bottom: 5px; }
        QDialog { background-color: #2b2b2b; color: #ffffff; }
    )");

  MainWindow window;
  window.show();

  return app.exec();
}
