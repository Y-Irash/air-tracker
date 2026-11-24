#pragma once

#include <map>
#include <set>
#include <utility>
#include <vector>

#include <QMainWindow>
#include <QString>

#include "core/point.h"
#include "core/tracker.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;
class QTimer;

class RadarWidget;

// Тонкое окно: только вызовы ядра и работа с виджетами, расчётов тут нет.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onLoadClicked();
    void onSaveClicked();
    void onStepClicked();
    void onStartClicked();
    void onStopClicked();
    void onResetClicked();
    void onTick();
    void onRangeChanged(int index);
    void onSpeedChanged(int index);
    void onHeadingsToggled(bool checked);

private:
    void buildUi();
    void loadFromPath(const QString &path);
    void refreshViews();
    void refreshStatus();
    void rememberTrail();
    void clearTrail();
    void reportConflicts();
    void log(const QString &message);

    double speedMultiplier() const;

    Tracker m_tracker;
    QString m_lastPath;
    double m_baseDtHours{0.2 / 3600.0};   // шаг таймера 200 мс в часах
    double m_modelSeconds{0.0};
    int m_conflictCount{0};

    std::map<int, std::vector<Point2D>> m_trails;
    std::set<std::pair<int, int>> m_activeConflicts;

    RadarWidget *m_radar{nullptr};
    QTableWidget *m_table{nullptr};
    QPlainTextEdit *m_log{nullptr};
    QLabel *m_status{nullptr};
    QTimer *m_timer{nullptr};
    QPushButton *m_startButton{nullptr};
    QPushButton *m_stopButton{nullptr};
    QComboBox *m_speedBox{nullptr};
    QComboBox *m_rangeBox{nullptr};
    QCheckBox *m_headingsBox{nullptr};
};
