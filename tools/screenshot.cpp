// Утилита для скриншотов README: собирает окно и рисует его в PNG без экрана.
// Запуск (из папки сборки, где лежит data/):
//   set QT_QPA_PLATFORM=offscreen
//   screenshot_tool.exe ..\docs

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QMetaObject>
#include <QPixmap>
#include <QString>
#include <QTimer>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QString outDir = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(".");
    QDir().mkpath(outDir);

    MainWindow window;

    // Выставить скорость 30× и дальность 100 км, чтобы на снимке были видны трассы.
    if (auto *speed = window.findChild<QComboBox *>(QStringLiteral("speedBox"))) {
        speed->setCurrentIndex(4);   // 30×
    }

    // Прокрутить модель, чтобы появились трассы, стрелки курса и сближение.
    for (int step = 0; step < 45; ++step) {
        QMetaObject::invokeMethod(&window, "onStepClicked", Qt::DirectConnection);
    }

    window.resize(1280, 800);
    window.show();

    // даём Qt разложить виджеты и один раз перерисовать
    QTimer::singleShot(400, [&window, &outDir, &app]() {
        const bool ok = window.grab().save(outDir + QStringLiteral("/screenshot.png"));
        if (!ok) {
            return;
        }
        app.quit();
    });

    return app.exec();
}
