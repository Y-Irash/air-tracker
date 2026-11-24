#include "mainwindow.h"

#include <algorithm>
#include <cmath>
#include <exception>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "core/conflict.h"
#include "core/io.h"
#include "radarwidget.h"

namespace {

const int kTrailLength = 40;              // сколько последних позиций помнить
const double kConflictThresholdKm = 10.0;

QString targetCell(double value)
{
    // три знака после запятой: в таблице радара этого достаточно,
    // а научная запись вида 1.07e-15 только мешает читать
    if (std::fabs(value) < 0.0005) {
        value = 0.0;
    }
    return QString::number(value, 'f', 3);
}

QString pairKey(int a, int b)
{
    return QStringLiteral("цели %1 и %2").arg(std::min(a, b)).arg(std::max(a, b));
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();

    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTick);

    // горячие клавиши: пробел — старт/стоп, S — шаг, R — сброс
    auto *toggleShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(toggleShortcut, &QShortcut::activated, this, [this]() {
        if (m_timer->isActive()) {
            onStopClicked();
        } else {
            onStartClicked();
        }
    });
    auto *stepShortcut = new QShortcut(QKeySequence(Qt::Key_S), this);
    connect(stepShortcut, &QShortcut::activated, this, &MainWindow::onStepClicked);
    auto *resetShortcut = new QShortcut(QKeySequence(Qt::Key_R), this);
    connect(resetShortcut, &QShortcut::activated, this, &MainWindow::onResetClicked);

    setWindowTitle(QStringLiteral("Трекер воздушной обстановки"));
    resize(1150, 680);

    // стартовый файл: data/ лежит рядом с exe благодаря CMake
    loadFromPath(QStringLiteral("data/sample.csv"));
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);

    // --- панель кнопок и настроек ---
    auto *top = new QHBoxLayout();
    auto *loadButton = new QPushButton(QStringLiteral("Загрузить CSV"), central);
    auto *saveButton = new QPushButton(QStringLiteral("Сохранить CSV"), central);
    auto *stepButton = new QPushButton(QStringLiteral("Шаг (S)"), central);
    m_startButton = new QPushButton(QStringLiteral("Старт (Space)"), central);
    m_stopButton = new QPushButton(QStringLiteral("Стоп (Space)"), central);
    auto *resetButton = new QPushButton(QStringLiteral("Сброс (R)"), central);
    m_stopButton->setEnabled(false);

    top->addWidget(loadButton);
    top->addWidget(saveButton);
    top->addWidget(stepButton);
    top->addWidget(m_startButton);
    top->addWidget(m_stopButton);
    top->addWidget(resetButton);

    top->addSpacing(14);
    top->addWidget(new QLabel(QStringLiteral("скорость:"), central));
    m_speedBox = new QComboBox(central);
    m_speedBox->addItem(QStringLiteral("1× (реальное)"), 1.0);
    m_speedBox->addItem(QStringLiteral("2×"), 2.0);
    m_speedBox->addItem(QStringLiteral("5×"), 5.0);
    m_speedBox->addItem(QStringLiteral("10×"), 10.0);
    m_speedBox->addItem(QStringLiteral("30×"), 30.0);
    m_speedBox->addItem(QStringLiteral("60× (1 мин в секунду)"), 60.0);
    m_speedBox->setCurrentIndex(0);
    m_speedBox->setObjectName(QStringLiteral("speedBox"));
    top->addWidget(m_speedBox);

    top->addSpacing(14);
    top->addWidget(new QLabel(QStringLiteral("дальность:"), central));
    m_rangeBox = new QComboBox(central);
    m_rangeBox->addItem(QStringLiteral("50 км"), 50.0);
    m_rangeBox->addItem(QStringLiteral("100 км"), 100.0);
    m_rangeBox->addItem(QStringLiteral("200 км"), 200.0);
    m_rangeBox->addItem(QStringLiteral("500 км"), 500.0);
    m_rangeBox->setCurrentIndex(1);
    m_rangeBox->setObjectName(QStringLiteral("rangeBox"));
    top->addWidget(m_rangeBox);

    m_headingsBox = new QCheckBox(QStringLiteral("векторы курса"), central);
    m_headingsBox->setChecked(true);
    top->addWidget(m_headingsBox);
    top->addStretch(1);

    root->addLayout(top);

    // --- радар + таблица + лог ---
    m_radar = new RadarWidget(central);
    m_radar->setTracker(&m_tracker);
    m_radar->setMaxRangeKm(100.0);

    m_table = new QTableWidget(central);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels(QStringList{QStringLiteral("id"),
                                                   QStringLiteral("тип"),
                                                   QStringLiteral("x, км"),
                                                   QStringLiteral("y, км"),
                                                   QStringLiteral("высота, км"),
                                                   QStringLiteral("скорость, км/ч"),
                                                   QStringLiteral("курс, °"),
                                                   QStringLiteral("до радара, км")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    // фиксированные ширины колонок: иначе после resizeColumnsToContents
    // таблица уезжает в горизонтальную прокрутку
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    const int widths[8] = {36, 74, 58, 58, 58, 70, 48, 70};
    for (int column = 0; column < 8; ++column) {
        m_table->setColumnWidth(column, widths[column]);
    }
    m_log = new QPlainTextEdit(central);
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(500);
    m_log->setMinimumHeight(90);

    auto *right = new QSplitter(Qt::Vertical, central);
    right->addWidget(m_table);
    right->addWidget(m_log);

    auto *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->addWidget(m_radar);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({740, 520});

    root->addWidget(splitter, 1);

    m_status = new QLabel(central);
    root->addWidget(m_status);

    setCentralWidget(central);

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadClicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(stepButton, &QPushButton::clicked, this, &MainWindow::onStepClicked);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(m_rangeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onRangeChanged);
    connect(m_speedBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSpeedChanged);
    connect(m_headingsBox, &QCheckBox::toggled, this, &MainWindow::onHeadingsToggled);
    connect(m_radar, &RadarWidget::targetClicked, this, [this](int id) {
        for (int row = 0; row < m_table->rowCount(); ++row) {
            if (m_table->item(row, 0) != nullptr && m_table->item(row, 0)->text().toInt() == id) {
                m_table->selectRow(row);
                log(QStringLiteral("выбрана цель %1").arg(id));
                break;
            }
        }
    });
}

double MainWindow::speedMultiplier() const
{
    if (m_speedBox == nullptr) {
        return 1.0;
    }
    return m_speedBox->currentData().toDouble();
}

void MainWindow::log(const QString &message)
{
    m_log->appendPlainText(message);
}

void MainWindow::clearTrail()
{
    m_trails.clear();
    m_activeConflicts.clear();
    m_conflictCount = 0;
    m_modelSeconds = 0.0;
    m_radar->setTrails(m_trails);
}

void MainWindow::rememberTrail()
{
    for (const Target &target : m_tracker.targets()) {
        std::vector<Point2D> &trail = m_trails[target.id()];
        trail.push_back(target.position());
        if (trail.size() > static_cast<std::size_t>(kTrailLength)) {
            trail.erase(trail.begin());
        }
    }
    m_radar->setTrails(m_trails);
}

void MainWindow::reportConflicts()
{
    std::set<std::pair<int, int>> current;
    const std::vector<Conflict> conflicts = findConflicts(m_tracker, kConflictThresholdKm);
    for (const Conflict &conflict : conflicts) {
        current.insert({std::min(conflict.idA, conflict.idB),
                        std::max(conflict.idA, conflict.idB)});
    }

    // сообщение пишем только при появлении и при прекращении сближения,
    // иначе лог забивается одинаковыми строками каждый тик
    for (const std::pair<int, int> &pair : current) {
        if (m_activeConflicts.count(pair) == 0) {
            double distanceKm = 0.0;
            for (const Conflict &conflict : conflicts) {
                if (std::min(conflict.idA, conflict.idB) == pair.first
                    && std::max(conflict.idA, conflict.idB) == pair.second) {
                    distanceKm = conflict.distanceKm;
                }
            }
            log(QStringLiteral("⚠ сближение: %1 — %2 км")
                    .arg(pairKey(pair.first, pair.second))
                    .arg(distanceKm, 0, 'f', 1));
        }
    }
    for (const std::pair<int, int> &pair : m_activeConflicts) {
        if (current.count(pair) == 0) {
            log(QStringLiteral("сближение закончилось: %1").arg(pairKey(pair.first, pair.second)));
        }
    }

    m_activeConflicts = current;
    m_conflictCount = static_cast<int>(conflicts.size());
}

void MainWindow::loadFromPath(const QString &path)
{
    try {
        const std::vector<TargetData> data = loadTargets(path.toStdString());

        m_tracker = Tracker{};
        for (const Target &target : toTargets(data)) {
            m_tracker.add(target);
        }

        m_lastPath = path;
        log(QStringLiteral("загружено %1 целей из %2").arg(m_tracker.size()).arg(path));
        clearTrail();
        refreshViews();
        reportConflicts();
    } catch (const std::exception &error) {
        const QString reason = QString::fromUtf8(error.what());
        log(QStringLiteral("ошибка загрузки: %1").arg(reason));
        QMessageBox::warning(this, QStringLiteral("Не удалось загрузить данные"), reason);
    }
}

void MainWindow::refreshViews()
{
    const std::vector<Target> &targets = m_tracker.targets();

    m_table->setRowCount(static_cast<int>(targets.size()));
    for (int row = 0; row < static_cast<int>(targets.size()); ++row) {
        const Target &target = targets[static_cast<std::size_t>(row)];
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(target.id())));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(target.type())));
        m_table->setItem(row, 2, new QTableWidgetItem(targetCell(target.position().x)));
        m_table->setItem(row, 3, new QTableWidgetItem(targetCell(target.position().y)));
        m_table->setItem(row, 4, new QTableWidgetItem(targetCell(target.altitudeKm())));
        m_table->setItem(row, 5, new QTableWidgetItem(targetCell(target.speedKmh())));
        m_table->setItem(row, 6, new QTableWidgetItem(targetCell(target.headingDeg())));
        m_table->setItem(row, 7,
                         new QTableWidgetItem(targetCell(distance({0, 0}, target.position()))));
    }

    // ширины колонок задаются один раз в buildUi(): пересчёт по содержимому
    // уводил таблицу в горизонтальную прокрутку
    m_radar->update();
    refreshStatus();
}

void MainWindow::refreshStatus()
{
    const int minutes = static_cast<int>(m_modelSeconds) / 60;
    const int seconds = static_cast<int>(m_modelSeconds) % 60;

    m_status->setText(QStringLiteral("целей: %1   ·   сближений: %2   ·   модельное время: %3:%4"
                                     "   ·   скорость: %5×   ·   дальность: %6 км")
                          .arg(m_tracker.size())
                          .arg(m_conflictCount)
                          .arg(minutes, 2, 10, QLatin1Char('0'))
                          .arg(seconds, 2, 10, QLatin1Char('0'))
                          .arg(speedMultiplier(), 0, 'g', 3)
                          .arg(m_radar->maxRangeKm(), 0, 'g', 3));
}

void MainWindow::onLoadClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Выберите файл целей"), QStringLiteral("data"),
        QStringLiteral("CSV (*.csv);;Все файлы (*)"));
    if (path.isEmpty()) {
        return;
    }
    loadFromPath(path);
}

void MainWindow::onSaveClicked()
{
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Куда сохранить цели"), QStringLiteral("targets_out.csv"),
        QStringLiteral("CSV (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    try {
        std::vector<TargetData> data;
        data.reserve(m_tracker.size());
        for (const Target &target : m_tracker.targets()) {
            TargetData item;
            item.id = target.id();
            item.type = target.type();
            item.position = target.position();
            item.altitude = target.altitudeKm();
            item.speedKmh = target.speedKmh();
            item.headingDeg = target.headingDeg();
            data.push_back(item);
        }
        saveTargets(data, path.toStdString());
        log(QStringLiteral("сохранено %1 целей в %2").arg(data.size()).arg(path));
    } catch (const std::exception &error) {
        const QString reason = QString::fromUtf8(error.what());
        log(QStringLiteral("ошибка сохранения: %1").arg(reason));
        QMessageBox::warning(this, QStringLiteral("Не удалось сохранить"), reason);
    }
}

void MainWindow::onStepClicked()
{
    m_tracker.updateAll(m_baseDtHours * speedMultiplier());
    m_modelSeconds += 0.2 * speedMultiplier();
    rememberTrail();
    refreshViews();
    reportConflicts();
}

void MainWindow::onStartClicked()
{
    if (m_timer->isActive()) {
        return;
    }
    m_timer->start();
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    log(QStringLiteral("движение включено (шаг 200 мс, скорость %1×)").arg(speedMultiplier()));
}

void MainWindow::onStopClicked()
{
    m_timer->stop();
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    log(QStringLiteral("движение остановлено"));
}

void MainWindow::onResetClicked()
{
    m_timer->stop();
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);

    const QString path = m_lastPath.isEmpty() ? QStringLiteral("data/sample.csv") : m_lastPath;
    log(QStringLiteral("сброс: перечитываю %1").arg(path));
    loadFromPath(path);
}

void MainWindow::onTick()
{
    onStepClicked();
}

void MainWindow::onRangeChanged(int index)
{
    Q_UNUSED(index);
    m_radar->setMaxRangeKm(m_rangeBox->currentData().toDouble());
    refreshStatus();
}

void MainWindow::onSpeedChanged(int index)
{
    Q_UNUSED(index);
    refreshStatus();
}

void MainWindow::onHeadingsToggled(bool checked)
{
    m_radar->setShowHeadings(checked);
}
