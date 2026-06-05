#include "MainWindow.h"
#include <QApplication>
int main(int argc,char* argv[]){QApplication app(argc,argv); if(QStringList(app.arguments()).contains("--self-test")) return 0; MainWindow window; window.show(); return app.exec();}
