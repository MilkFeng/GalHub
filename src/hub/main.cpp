#include <iostream>
#include <QApplication>

#include "env_manager.h"
#include "gui/main_window.h"

using namespace std;

int main (int argc, char *argv[]) {
    try {
        EnvManager::instance().read_config();
        EnvManager::instance().init_env();

        QApplication app(argc, argv);
        app.setStyle("Fusion");

        MainWindow w;
        w.show();
        return app.exec();
    } catch (const std::exception &e) {
        cerr << "Error: " << e.what() << endl;
        MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
        return 1;
    } catch (...) {
        cerr << "Unknown error" << endl;
        MessageBoxA(nullptr, "Unknown error", "Error", MB_ICONERROR);
        return 1;
    }
}