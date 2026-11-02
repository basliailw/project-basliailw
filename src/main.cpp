#include "aiws/gui/main_window.hpp"

#include <QApplication>
#include <QMessageBox>

// TODO: Define the ExitCallback function used to confirm an Exit request.
void confirm_exit(QWidget* parent) {
    QMessageBox::StandardButton answer = QMessageBox::question(
        parent, "Exit", "Are you sure you want to exit?",
        QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QApplication::quit();
}

// M4 addition: reads --build-delay-ms N, returns 0 if missing
int read_build_delay_ms(const QStringList& args) {
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--build-delay-ms" && i + 1 < args.size())
            return args[i + 1].toInt();
        if (args[i].startsWith("--build-delay-ms="))
            return args[i].mid(QString("--build-delay-ms=").size()).toInt();
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // TODO: Create the QApplication, construct/configure the MainWindow,
    // register the ExitCallback, show the window, and run the event loop.
    QApplication app(argc, argv);
    aiws::gui::MainWindow window;
    window.set_exit_callback(&confirm_exit);

    window.set_build_delay_ms(read_build_delay_ms(QApplication::arguments()));  // m4 addition

    window.show();
    return app.exec();
}
