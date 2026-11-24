#pragma once

#include <map>
#include <vector>

#include <QWidget>

#include "core/point.h"
#include "core/tracker.h"

// Индикатор кругового обзора. Данные берёт из ядра, экранные координаты не хранит:
// масштаб и центр считаются заново при каждой перерисовке.
class RadarWidget : public QWidget {
    Q_OBJECT
public:
    explicit RadarWidget(QWidget *parent = nullptr);

    // Не владеет списком целей — только смотрит.
    void setTracker(const Tracker *tracker);

    // Трассы позиций: история приходит от окна, виджет её только рисует.
    void setTrails(const std::map<int, std::vector<Point2D>> &trails);

    void setMaxRangeKm(double km);
    double maxRangeKm() const;

    void setShowHeadings(bool show);
    bool showHeadings() const;

signals:
    // Свой сигнал: клик по точке цели на радаре (id), чтобы окно выделило строку.
    void targetClicked(int id);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QPointF toScreen(const Point2D &point, const QPointF &center, double radiusPx) const;
    void drawHeadings(QPainter &painter, const QPointF &center, double radiusPx) const;
    void drawTrails(QPainter &painter, const QPointF &center, double radiusPx) const;

    const Tracker *m_tracker{nullptr};
    std::map<int, std::vector<Point2D>> m_trails;
    double m_maxRangeKm{100.0};
    bool m_showHeadings{true};
};
