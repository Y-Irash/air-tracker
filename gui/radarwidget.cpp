#include "radarwidget.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <QLineF>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QString>

#include "core/conflict.h"

namespace {

const double kConflictThresholdKm = 10.0;
const int kRings = 4;

QString rangeLabel(int ring, double maxRangeKm)
{
    const double km = maxRangeKm * ring / kRings;
    return QString::number(km, 'g', 3) + QStringLiteral(" км");
}

} // namespace

RadarWidget::RadarWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(320, 320);
    setAutoFillBackground(false);
}

void RadarWidget::setTracker(const Tracker *tracker)
{
    m_tracker = tracker;
    update();
}

void RadarWidget::setTrails(const std::map<int, std::vector<Point2D>> &trails)
{
    m_trails = trails;
    update();
}

void RadarWidget::setMaxRangeKm(double km)
{
    if (km > 0.0) {
        m_maxRangeKm = km;
        update();
    }
}

double RadarWidget::maxRangeKm() const
{
    return m_maxRangeKm;
}

void RadarWidget::setShowHeadings(bool show)
{
    m_showHeadings = show;
    update();
}

bool RadarWidget::showHeadings() const
{
    return m_showHeadings;
}

QPointF RadarWidget::toScreen(const Point2D &point, const QPointF &center, double radiusPx) const
{
    const double scale = radiusPx / m_maxRangeKm;
    // инверсия Y: на экране Y растёт вниз, а в ядре — на север
    return QPointF(center.x() + point.x * scale, center.y() - point.y * scale);
}

void RadarWidget::drawTrails(QPainter &painter, const QPointF &center, double radiusPx) const
{
    for (const auto &entry : m_trails) {
        const std::vector<Point2D> &trail = entry.second;
        if (trail.size() < 2) {
            continue;
        }

        painter.setBrush(Qt::NoBrush);
        // старые точки бледнее новых: альфа растёт к концу трассы
        for (std::size_t i = 1; i < trail.size(); ++i) {
            // сегменты за пределами дальности радара не рисуем: цель «не видна»
            if (distance({0, 0}, trail[i - 1]) > m_maxRangeKm
                || distance({0, 0}, trail[i]) > m_maxRangeKm) {
                continue;
            }
            const int alpha = 30 + static_cast<int>(120.0 * i / trail.size());
            painter.setPen(QPen(QColor(90, 180, 130, alpha), 1.4));
            painter.drawLine(toScreen(trail[i - 1], center, radiusPx),
                             toScreen(trail[i], center, radiusPx));
        }
    }
}

void RadarWidget::drawHeadings(QPainter &painter, const QPointF &center, double radiusPx) const
{
    if (m_tracker == nullptr || !m_showHeadings) {
        return;
    }

    for (const Target &target : m_tracker->targets()) {
        const QPointF start = toScreen(target.position(), center, radiusPx);

        // длина вектора — от скорости: быстрая цель показывает более длинную стрелку
        const double lengthPx = std::min(34.0, 8.0 + target.speedKmh() * 0.03);
        const double rad = degreesToRadians(target.headingDeg());
        const QPointF end(start.x() + lengthPx * std::sin(rad),
                          start.y() - lengthPx * std::cos(rad));

        painter.setPen(QPen(QColor(120, 200, 255, 170), 1.6));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(start, end);

        // наконечник стрелки
        const double arrow = 5.0;
        const double back = degreesToRadians(target.headingDeg() + 180.0);
        const QPointF left(end.x() + arrow * std::sin(back + degreesToRadians(28.0)),
                           end.y() - arrow * std::cos(back + degreesToRadians(28.0)));
        const QPointF right(end.x() + arrow * std::sin(back - degreesToRadians(28.0)),
                            end.y() - arrow * std::cos(back - degreesToRadians(28.0)));

        QPolygonF head;
        head << end << left << right;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(120, 200, 255, 190));
        painter.drawPolygon(head);
    }
}

void RadarWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_tracker == nullptr) {
        return;
    }

    const QPointF center(width() / 2.0, height() / 2.0);
    const double radius = std::min(width(), height()) / 2.0 - 10.0;
    if (radius <= 0.0) {
        return;
    }

    // ищем ближайшую цель в пределах 14 пикселей от точки клика
    int bestId = -1;
    double bestDistance = 14.0;
    for (const Target &target : m_tracker->targets()) {
        const QPointF screen = toScreen(target.position(), center, radius);
        const double d = QLineF(screen, event->pos()).length();
        if (d < bestDistance) {
            bestDistance = d;
            bestId = target.id();
        }
    }

    if (bestId >= 0) {
        emit targetClicked(bestId);
    }
}

void RadarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // фон
    painter.fillRect(rect(), QColor(12, 18, 28));

    const QPointF center(width() / 2.0, height() / 2.0);
    const double radius = std::min(width(), height()) / 2.0 - 10.0;
    if (radius <= 0.0) {
        return;
    }

    // кольца дальности
    painter.setPen(QPen(QColor(38, 66, 96), 1));
    for (int ring = 1; ring <= kRings; ++ring) {
        const double ringRadius = radius * ring / kRings;
        painter.drawEllipse(center, ringRadius, ringRadius);
    }

    // лучи азимутов через 30°
    painter.setPen(QPen(QColor(28, 52, 76), 1));
    for (int angle = 0; angle < 360; angle += 30) {
        const double rad = degreesToRadians(angle);
        const QPointF edge(center.x() + radius * std::sin(rad),
                           center.y() - radius * std::cos(rad));
        painter.drawLine(center, edge);
    }

    // подписи сторон света
    painter.setPen(QPen(QColor(120, 150, 180), 1));
    painter.drawText(QPointF(center.x() - 4, center.y() - radius + 12), QStringLiteral("С"));
    painter.drawText(QPointF(center.x() + radius - 14, center.y() + 4), QStringLiteral("В"));
    painter.drawText(QPointF(center.x() - 4, center.y() + radius - 4), QStringLiteral("Ю"));
    painter.drawText(QPointF(center.x() - radius + 6, center.y() + 4), QStringLiteral("З"));

    // подписи дальности по оси на восток
    for (int ring = 1; ring <= kRings; ++ring) {
        const double ringRadius = radius * ring / kRings;
        painter.setPen(QPen(QColor(85, 110, 140), 1));
        painter.drawText(QPointF(center.x() + ringRadius + 4, center.y() - 6),
                         rangeLabel(ring, m_maxRangeKm));
    }

    if (m_tracker == nullptr) {
        painter.setPen(QPen(QColor(120, 140, 165), 1));
        painter.drawText(QRectF(0, 0, width(), height()), Qt::AlignCenter,
                         QStringLiteral("нажмите «Загрузить CSV»"));
        return;
    }

    drawTrails(painter, center, radius);
    drawHeadings(painter, center, radius);

    // какие цели в сближении — считает ядро, виджет только подсвечивает
    std::vector<int> conflictedIds;
    for (const Conflict &conflict : findConflicts(*m_tracker, kConflictThresholdKm)) {
        conflictedIds.push_back(conflict.idA);
        conflictedIds.push_back(conflict.idB);
    }

    for (const Target &target : m_tracker->targets()) {
        // за пределами дальности цели не видно — как на настоящем индикаторе
        if (distance({0, 0}, target.position()) > m_maxRangeKm) {
            continue;
        }

        const QPointF screen = toScreen(target.position(), center, radius);
        const bool hot = std::find(conflictedIds.begin(), conflictedIds.end(), target.id())
                         != conflictedIds.end();

        painter.setPen(Qt::NoPen);
        painter.setBrush(hot ? QColor(248, 81, 73) : QColor(57, 211, 83));
        painter.drawEllipse(screen, 4.5, 4.5);

        painter.setPen(QPen(hot ? QColor(255, 138, 128) : QColor(150, 200, 160), 1));
        painter.drawText(QPointF(screen.x() + 8, screen.y() - 6), QString::number(target.id()));
    }
}
