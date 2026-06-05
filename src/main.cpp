#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QPixmap>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  const QStringList arguments = app.arguments();
  if (arguments.contains("--self-test")) {
    return 0;
  }
  if (arguments.contains("--screenshot")) {
    MainWindow window;
    window.resize(1280, 760);
    window.show();
    app.processEvents();
    const QString path = arguments.value(arguments.indexOf("--screenshot") + 1, "premium-content-radar-preview.png");
    QPixmap pixmap = window.grab();
    return pixmap.save(path) ? 0 : 7;
  }
  MainWindow window;
  window.show();
  return app.exec();
}
