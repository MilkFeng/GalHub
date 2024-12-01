#include <QApplication>

#include "env_manager.h"
#include "gui/main_window.h"

int main (int argc, char *argv[]) {
    EnvManager::instance().read_config();
    EnvManager::instance().init_env();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}