// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/LiveAnalyzer.h"

#include <algorithm>
#include <functional>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/WaveformRecorder.h"

// ============================================================================
// AnalyzerCanvas — scrollable signal traces
// ============================================================================

AnalyzerCanvas::AnalyzerCanvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(AnalyzerLayout::TraceHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void AnalyzerCanvas::setPixelsPerNs(double ppn)
{
    m_pixelsPerNs = qBound(1e-7, ppn, 10.0);
    updateGeometry();
    update();
    emit zoomChanged();
}

void AnalyzerCanvas::zoomIn()
{
    setPixelsPerNs(m_pixelsPerNs * 2.0);
}

void AnalyzerCanvas::zoomOut()
{
    setPixelsPerNs(m_pixelsPerNs / 2.0);
}

QSize AnalyzerCanvas::sizeHint() const
{
    const int traceCount = m_recorder ? m_recorder->traceCount() : 0;
    const int h = traceCount * (AnalyzerLayout::TraceHeight + AnalyzerLayout::TraceMargin);

    int w = 400;
    if (m_recorder && m_recorder->maxTime() > 0) {
        // Qt hard-caps widget dimensions at QWIDGETSIZE_MAX; a minimum size beyond it is
        // rejected with a qWarning on EVERY setMinimumSize() call and re-invalidates layout,
        // so a long recording at high zoom must clamp here (timeOrigin() then slides the
        // canvas window over the newest data instead of growing the widget).
        const double pixels = static_cast<double>(m_recorder->maxTime()) * m_pixelsPerNs;
        w = static_cast<int>(qMin(pixels, static_cast<double>(QWIDGETSIZE_MAX - 20))) + 20;
    }
    return {qBound(100, w, QWIDGETSIZE_MAX),
            qBound(AnalyzerLayout::TraceHeight, h, QWIDGETSIZE_MAX)};
}

QSize AnalyzerCanvas::minimumSizeHint() const
{
    return {100, AnalyzerLayout::TraceHeight};
}

SimTime AnalyzerCanvas::timeOrigin() const
{
    if (!m_recorder || m_pixelsPerNs <= 0) {
        return 0;
    }
    const SimTime maxTime = m_recorder->maxTime();
    const double capPixels = static_cast<double>(QWIDGETSIZE_MAX - 20);
    if (static_cast<double>(maxTime) * m_pixelsPerNs <= capPixels) {
        return 0;
    }
    return maxTime - static_cast<SimTime>(capPixels / m_pixelsPerNs);
}

// ============================================================================
// Paint
// ============================================================================

void AnalyzerCanvas::paintEvent(QPaintEvent *event)
{
    // Resize to fit content so the parent QScrollArea shows scrollbars. sizeHint() is
    // clamped to QWIDGETSIZE_MAX, so this settles instead of re-requesting an over-limit
    // minimum (which Qt rejects with a qWarning) on every repaint. The explicit resize()
    // covers the SHRINK direction (zoom-out, Fit): a smaller minimum alone never shrinks
    // an already-grown canvas — the scroll area immediately re-expands to the viewport if
    // the hint is narrower than it.
    const QSize hint = sizeHint();
    if (hint != size()) {
        setMinimumSize(hint);
        resize(hint);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(event->rect(), palette().base());

    if (!m_recorder || m_recorder->traceCount() == 0) {
        painter.setPen(palette().color(QPalette::Disabled, QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter, tr("No signals watched"));
        return;
    }

    const int traceWidth = width();
    if (traceWidth <= 0 || m_pixelsPerNs <= 0) {
        return;
    }

    // Convert the exposed paint region to a sim-time window so drawing costs O(viewport)
    // instead of O(recorded history). The margin absorbs the 2px trace edges anchored just
    // outside the exposed rect, keeping partial-expose repaints seamless.
    constexpr int kExposeMargin = 120;
    const SimTime origin = timeOrigin();
    const int exposedLeft = qMax(0, event->rect().left() - kExposeMargin);
    const int exposedRight = qMin(traceWidth, event->rect().right() + kExposeMargin);
    const SimTime visibleStart = origin + static_cast<SimTime>(exposedLeft / m_pixelsPerNs);
    const SimTime visibleEnd = origin + static_cast<SimTime>(exposedRight / m_pixelsPerNs);

    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = i * (AnalyzerLayout::TraceHeight + AnalyzerLayout::TraceMargin);
        if (y > event->rect().bottom() || y + AnalyzerLayout::TraceHeight < event->rect().top()) {
            continue; // row entirely outside the exposed region
        }
        drawTrace(painter, i, y, traceWidth, AnalyzerLayout::TraceHeight, origin, visibleStart, visibleEnd);
    }
}

// ============================================================================
// Signal trace
// ============================================================================

void AnalyzerCanvas::drawTrace(QPainter &painter, int traceIndex, int y0,
                               int traceWidth, int height,
                               SimTime origin, SimTime visibleStart, SimTime visibleEnd)
{
    const auto &trace = m_recorder->trace(traceIndex);
    const auto &transitions = trace.transitions;

    painter.setPen(QPen(palette().color(QPalette::Mid), 1, Qt::DotLine));
    painter.drawLine(0, y0 + height, traceWidth, y0 + height);

    if (transitions.isEmpty()) {
        painter.setPen(QPen(QColor(100, 100, 100), 2));
        painter.drawLine(0, y0 + LOW_Y_OFFSET, traceWidth, y0 + LOW_Y_OFFSET);
        return;
    }

    painter.setPen(QPen(AnalyzerLayout::traceColor(traceIndex), 2));

    auto yForStatus = [&](Status s) -> int {
        return y0 + (s == Status::Active ? HIGH_Y_OFFSET : LOW_Y_OFFSET);
    };

    // Nothing is known before the trace's first recorded sample (watching typically starts
    // mid-run): leave that region blank — only the dotted row baseline — instead of
    // asserting a LOW that was never observed.
    const SimTime firstSample = transitions.first().first;
    if (visibleEnd < firstSample) {
        return;
    }

    // Signal level entering the window (or at the first sample if the window starts before
    // it), then only the transitions inside the window — located by binary search, NOT a
    // scan from t = 0, so a repaint touches O(log n + visible) of a history that can hold
    // hours of transitions.
    SimTime currentTime = qMax(visibleStart, firstSample);
    Status currentStatus = trace.statusAt(currentTime);

    const auto firstVisible = std::lower_bound(
        transitions.cbegin(), transitions.cend(), currentTime,
        [](const QPair<SimTime, Status> &tr, SimTime t) { return tr.first < t; });

    for (auto it = firstVisible; it != transitions.cend(); ++it) {
        const auto &[tTime, tStatus] = *it;
        if (tTime > visibleEnd) {
            break;
        }

        const int xFrom = timeToX(currentTime, origin);
        const int xTo = timeToX(tTime, origin);
        const int yCurrent = yForStatus(currentStatus);

        if (xTo > xFrom) {
            painter.drawLine(xFrom, yCurrent, xTo, yCurrent);
        }

        if (tStatus != currentStatus) {
            const int yNew = yForStatus(tStatus);
            painter.drawLine(xTo, yCurrent, xTo, yNew);
        }

        currentStatus = tStatus;
        currentTime = tTime;
    }

    // Extend the last known level to the window's right edge (clamped to the canvas).
    const int xFrom = timeToX(currentTime, origin);
    const int xEnd = qMin(traceWidth, timeToX(visibleEnd, origin));
    if (xEnd > xFrom) {
        painter.drawLine(xFrom, yForStatus(currentStatus), xEnd, yForStatus(currentStatus));
    }
}

// ============================================================================
// Coordinate conversion
// ============================================================================

int AnalyzerCanvas::timeToX(SimTime time, SimTime origin) const
{
    // SimTime is unsigned: subtract in the right order so a pre-origin time (e.g. a ruler
    // tick rounded down past the window start) maps to a negative x and gets culled, instead
    // of wrapping around to a huge positive coordinate.
    const double delta = (time >= origin) ? static_cast<double>(time - origin)
                                          : -static_cast<double>(origin - time);
    return static_cast<int>(qBound(-1e8, delta * m_pixelsPerNs, 1e8));
}

// ============================================================================
// Mouse wheel zoom
// ============================================================================

void AnalyzerCanvas::wheelEvent(QWheelEvent *event)
{
    // Any wheel zooms, no modifier — parity with the stimulus-editor grid, whose event
    // filter consumes every wheel as zoom (the original beWavedDolphin behavior); the
    // scrollbars pan. Request, don't rescale: anchoring the zoom on the sim-time under
    // the cursor needs the scroll position and viewport, which only the panel knows.
    if (const int dy = event->angleDelta().y(); dy != 0) {
        emit zoomStepRequested(dy > 0 ? 1 : -1, event->position().x());
    }
    event->accept();
}

// ============================================================================
// AnalyzerTimeRuler — frozen ruler row
// ============================================================================

AnalyzerTimeRuler::AnalyzerTimeRuler(const AnalyzerCanvas *canvas, QWidget *parent)
    : QWidget(parent)
    , m_canvas(canvas)
{
    setFixedHeight(AnalyzerLayout::RulerHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void AnalyzerTimeRuler::setHorizontalOffset(int offset)
{
    if (m_horizontalOffset == offset) {
        return;
    }
    m_horizontalOffset = offset;
    update();
}

void AnalyzerTimeRuler::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), palette().window());
    painter.setPen(palette().color(QPalette::Text));

    const double ppn = m_canvas ? m_canvas->pixelsPerNs() : 0.0;
    if (ppn <= 0 || width() <= 0) {
        return;
    }

    // The visible sim-time window: the ruler's x = 0 sits at canvas x = m_horizontalOffset.
    const SimTime origin = m_canvas->timeOrigin();
    const SimTime visibleStart = origin + static_cast<SimTime>(m_horizontalOffset / ppn);
    const SimTime visibleEnd = origin + static_cast<SimTime>((m_horizontalOffset + width()) / ppn);

    if (visibleEnd <= visibleStart) {
        return;
    }

    const double targetPixelSpacing = 100.0;
    double rawInterval = targetPixelSpacing / ppn;

    double magnitude = 1.0;
    while (magnitude * 10 < rawInterval) { magnitude *= 10; }
    double niceInterval;
    if (rawInterval <= magnitude * 2) {
        niceInterval = magnitude * 2;
    } else if (rawInterval <= magnitude * 5) {
        niceInterval = magnitude * 5;
    } else {
        niceInterval = magnitude * 10;
    }

    const auto tickInterval = static_cast<SimTime>(qMax(niceInterval, 1.0));
    const SimTime firstTick = (visibleStart / tickInterval) * tickInterval;

    const char *unit = "ns";
    double unitDiv = 1.0;
    if (tickInterval >= 1'000'000) {
        unit = "ms";
        unitDiv = 1'000'000.0;
    } else if (tickInterval >= 1'000) {
        unit = "\xC2\xB5s";
        unitDiv = 1'000.0;
    }

    for (SimTime t = firstTick; t <= visibleEnd; t += tickInterval) {
        // Ruler-local x; firstTick can round below visibleStart, mapping to a culled x < 0.
        const double delta = (t >= origin) ? static_cast<double>(t - origin)
                                           : -static_cast<double>(origin - t);
        const int x = static_cast<int>(qBound(-1e8, delta * ppn, 1e8)) - m_horizontalOffset;
        if (x < 0 || x > width()) {
            continue;
        }
        painter.drawLine(x, AnalyzerLayout::RulerHeight - 6, x, AnalyzerLayout::RulerHeight);

        const QString label = QString::number(static_cast<double>(t) / unitDiv, 'g', 4) + " " + unit;
        painter.drawText(x + 2, AnalyzerLayout::RulerHeight - 8, label);
    }

    painter.drawLine(0, AnalyzerLayout::RulerHeight - 1, width(), AnalyzerLayout::RulerHeight - 1);
}

// ============================================================================
// AnalyzerLabelColumn — frozen signal-name column
// ============================================================================

AnalyzerLabelColumn::AnalyzerLabelColumn(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(AnalyzerLayout::LabelWidth);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

void AnalyzerLabelColumn::setVerticalOffset(int offset)
{
    if (m_verticalOffset == offset) {
        return;
    }
    m_verticalOffset = offset;
    update();
}

QSize AnalyzerLabelColumn::sizeHint() const
{
    // PREFERRED height only — the layout stretches the column to the viewport, and painting
    // is offset-driven. Capping at a handful of rows keeps a long watch list from ballooning
    // the whole tool's preferred size; the vertical scrollbar reaches the rest.
    const int traceCount = m_recorder ? qMin(m_recorder->traceCount(), 6) : 0;
    const int h = traceCount * (AnalyzerLayout::TraceHeight + AnalyzerLayout::TraceMargin);
    return {AnalyzerLayout::LabelWidth, qMax(h, AnalyzerLayout::TraceHeight)};
}

void AnalyzerLabelColumn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), palette().base());

    if (!m_recorder || m_recorder->traceCount() == 0) {
        return;
    }

    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = i * (AnalyzerLayout::TraceHeight + AnalyzerLayout::TraceMargin) - m_verticalOffset;
        if (y + AnalyzerLayout::TraceHeight < 0 || y > height()) {
            continue; // row scrolled out of view
        }

        painter.fillRect(0, y, AnalyzerLayout::LabelWidth - 2, AnalyzerLayout::TraceHeight, palette().alternateBase());
        painter.setPen(AnalyzerLayout::traceColor(i)); // name colored like its trace
        painter.drawText(QRect(4, y, AnalyzerLayout::LabelWidth - 8, AnalyzerLayout::TraceHeight),
                         Qt::AlignVCenter | Qt::AlignRight,
                         m_recorder->trace(i).name);
    }
}

// ============================================================================
// LiveAnalyzerPanel — watch controls + frozen-pane trace view
// ============================================================================

LiveAnalyzerPanel::LiveAnalyzerPanel(Scene *scene, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Toolbar row
    auto *toolbar = new QHBoxLayout();
    auto *btnWatch = new QPushButton(tr("Watch All"), this);
    auto *btnClear = new QPushButton(tr("Clear"), this);
    auto *btnZoomIn = new QPushButton(tr("+"), this);
    auto *btnZoomOut = new QPushButton(tr("-"), this);
    auto *btnFit = new QPushButton(tr("Fit"), this);
    auto *btnLatestEdge = new QPushButton(tr("Latest Edge"), this);
    btnWatch->setObjectName("analyzerWatchAllButton");
    btnClear->setObjectName("analyzerClearButton");
    btnZoomIn->setObjectName("analyzerZoomInButton");
    btnZoomOut->setObjectName("analyzerZoomOutButton");
    btnFit->setObjectName("analyzerFitButton");
    btnLatestEdge->setObjectName("analyzerLatestEdgeButton");
    btnWatch->setToolTip(tr("Watch all output signals in the circuit"));
    btnClear->setToolTip(tr("Remove all watched signals"));
    btnLatestEdge->setToolTip(tr("Zoom to the newest recorded transition at nanosecond scale — pause the simulation to hold the view"));
    toolbar->addWidget(btnWatch);
    toolbar->addWidget(btnClear);
    toolbar->addStretch();
    toolbar->addWidget(btnZoomOut);
    toolbar->addWidget(btnZoomIn);
    toolbar->addWidget(btnFit);
    toolbar->addWidget(btnLatestEdge);
    layout->addLayout(toolbar);

    // Frozen-pane grid: ruler row pinned on top, label column pinned on the left, traces in
    // the scrollable cell. The corner spacer keeps the ruler aligned with the canvas.
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_canvas = new AnalyzerCanvas(m_scrollArea);
    m_scrollArea->setWidget(m_canvas);

    m_ruler = new AnalyzerTimeRuler(m_canvas, this);
    m_labels = new AnalyzerLabelColumn(this);

    auto *corner = new QWidget(this);
    corner->setFixedSize(AnalyzerLayout::LabelWidth, AnalyzerLayout::RulerHeight);

    auto *grid = new QGridLayout();
    grid->setSpacing(0);
    grid->addWidget(corner, 0, 0);
    grid->addWidget(m_ruler, 0, 1);
    grid->addWidget(m_labels, 1, 0); // fills the cell: rows below the fold paint when scrolled
    grid->addWidget(m_scrollArea, 1, 1);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);
    layout->addLayout(grid);

    // Recorder binding is per-scene and stable for the panel's lifetime; the QPointer +
    // destroyed() hook below cover the scene dying first.
    m_canvas->setRecorder(recorder());
    m_labels->setRecorder(recorder());

    connect(btnWatch, &QPushButton::clicked, this, &LiveAnalyzerPanel::watchAllSignals);
    connect(btnClear, &QPushButton::clicked, this, &LiveAnalyzerPanel::clearWatchedSignals);
    connect(btnZoomIn, &QPushButton::clicked, this, &LiveAnalyzerPanel::zoomIn);
    connect(btnZoomOut, &QPushButton::clicked, this, &LiveAnalyzerPanel::zoomOut);
    connect(btnFit, &QPushButton::clicked, this, &LiveAnalyzerPanel::zoomFit);
    connect(btnLatestEdge, &QPushButton::clicked, this, &LiveAnalyzerPanel::zoomToLatestEdge);
    connect(m_canvas, &AnalyzerCanvas::zoomChanged, m_ruler, qOverload<>(&QWidget::update));

    // Wheel: zoom anchored on the sim-time under the cursor (the canvas only reports
    // the cursor position; the anchor math needs the scroll position, known here).
    connect(m_canvas, &AnalyzerCanvas::zoomStepRequested, this, [this](int direction, double canvasX) {
        const double anchorViewportX = canvasX - m_scrollArea->horizontalScrollBar()->value();
        const double factor = (direction > 0) ? 2.0 : 0.5;
        applyAnchoredZoom(m_canvas->pixelsPerNs() * factor, anchorViewportX);
    });

    // Frozen-pane sync: the ruler follows horizontal scrolling, the labels vertical.
    auto *hBar = m_scrollArea->horizontalScrollBar();
    auto *vBar = m_scrollArea->verticalScrollBar();
    connect(hBar, &QAbstractSlider::valueChanged, m_ruler, &AnalyzerTimeRuler::setHorizontalOffset);
    connect(vBar, &QAbstractSlider::valueChanged, m_labels, &AnalyzerLabelColumn::setVerticalOffset);

    // Follow the newest data with a sticky tail. The canvas grows during paint, which lands
    // as a rangeChanged on the scroll bar — pinning there can never miss a growth step.
    // Scrolling away from the right edge releases the pin so earlier (ns-scale) transitions
    // stay inspectable while the simulation runs; scrolling back to the edge re-engages it.
    connect(hBar, &QAbstractSlider::valueChanged, this, [this, hBar](int value) {
        m_followTail = (value >= hBar->maximum() - 4); // slack absorbs drag/wheel jitter
    });
    connect(hBar, &QAbstractSlider::rangeChanged, this, [this, hBar](int, int max) {
        if (m_followTail) {
            hBar->setValue(max);
        }
    });

    // Adapt the zoom whenever the simulation speed changes (temporal tick window).
    if (m_scene) {
        connect(m_scene->simulation(), &Simulation::timePerTickChanged,
                this, &LiveAnalyzerPanel::adaptZoomToTickWindow);
        connect(m_scene, &QObject::destroyed, this, [this] {
            m_canvas->setRecorder(nullptr);
            m_labels->setRecorder(nullptr);
            m_refreshTimer.stop();
        });
    }

    // Periodic repaint so new transitions appear as they record (started while visible).
    m_refreshTimer.setInterval(100);
    connect(&m_refreshTimer, &QTimer::timeout, this, [this] {
        m_labels->update();
        m_canvas->update();
        m_ruler->update();
    });
}

WaveformRecorder *LiveAnalyzerPanel::recorder() const
{
    return m_scene ? &m_scene->simulation()->waveformRecorder() : nullptr;
}

void LiveAnalyzerPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_refreshTimer.start();
}

void LiveAnalyzerPanel::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_refreshTimer.stop();
}

void LiveAnalyzerPanel::watchAllSignals()
{
    auto *rec = recorder();
    if (!rec) {
        return;
    }
    rec->clear();

    // Watch all elements that have logic and output ports.
    // Skip VCC/GND (static constants) and decorative elements (no logic).
    for (auto *elm : m_scene->elements()) {
        if (!elm || elm->outputSize() == 0) {
            continue;
        }
        // Skip VCC and GND — they never change.
        if (elm->elementType() == ElementType::InputVcc || elm->elementType() == ElementType::InputGnd) {
            continue;
        }
        const QString name = elm->label().isEmpty()
                                 ? ElementFactory::translatedName(elm->elementType())
                                 : elm->label();
        for (int port = 0; port < elm->outputSize(); ++port) {
            const QString portSuffix = (elm->outputSize() > 1)
                                           ? QString(" [%1]").arg(port)
                                           : QString();
            rec->watchSignal(name + portSuffix, elm, port);
        }
    }

    rec->setRecording(true);
    m_canvas->setRecorder(rec);
    m_labels->setRecorder(rec);

    adaptZoomToTickWindow(m_scene->simulation()->timePerTick());
    hintIfFunctionalMode();

    m_labels->updateGeometry();
    m_canvas->update();
    m_labels->update();
}

void LiveAnalyzerPanel::clearWatchedSignals()
{
    auto *rec = recorder();
    if (!rec) {
        return;
    }
    rec->setRecording(false);
    rec->clear();
    m_labels->updateGeometry();
    m_canvas->update();
    m_labels->update();
    m_ruler->update();
}

void LiveAnalyzerPanel::watchICInternals(IC *ic)
{
    auto *rec = recorder();
    if (!rec || !ic) {
        return;
    }

    // Recursively watch every internal primitive's output ports, with a path-prefixed name
    // ("<IC>/<sub-IC>/<element>") so nested signals stay distinguishable. Augments the current
    // watch set rather than clearing it, so several ICs can be opened side by side.
    const std::function<void(const QVector<GraphicElement *> &, const QString &)> watchRec =
        [&](const QVector<GraphicElement *> &elements, const QString &prefix) {
            for (auto *elm : elements) {
                if (!elm) {
                    continue;
                }
                const QString label = elm->label().isEmpty()
                                          ? ElementFactory::translatedName(elm->elementType())
                                          : elm->label();
                if (elm->elementType() == ElementType::IC) {
                    watchRec(static_cast<IC *>(elm)->internalElements(), prefix + label + QStringLiteral("/"));
                    continue;
                }
                if (elm->outputSize() == 0
                    || elm->elementType() == ElementType::InputVcc
                    || elm->elementType() == ElementType::InputGnd) {
                    continue;
                }
                for (int port = 0; port < elm->outputSize(); ++port) {
                    const QString portSuffix = (elm->outputSize() > 1) ? QStringLiteral(" [%1]").arg(port) : QString();
                    rec->watchSignal(prefix + label + portSuffix, elm, port);
                }
            }
        };

    const QString root = (ic->label().isEmpty() ? ElementFactory::translatedName(ElementType::IC) : ic->label())
                       + QStringLiteral("/");
    watchRec(ic->internalElements(), root);

    rec->setRecording(true);
    m_canvas->setRecorder(rec);
    m_labels->setRecorder(rec);

    hintIfFunctionalMode();

    m_labels->updateGeometry();
    m_canvas->update();
    m_labels->update();
}

void LiveAnalyzerPanel::settleCanvasGeometry()
{
    // grab() renders synchronously, driving the paint-time canvas resize that feeds the
    // scroll area's ranges (update() alone lands on the next event-loop cycle).
    m_canvas->grab(QRect(0, 0, 1, 1));
}

void LiveAnalyzerPanel::applyAnchoredZoom(double targetPpn, double anchorViewportX)
{
    auto *hBar = m_scrollArea->horizontalScrollBar();

    // The instant under the anchor point at the CURRENT scale (double: SimTime values are
    // far below 2^53, so the mantissa holds them exactly).
    const double anchorTime = static_cast<double>(m_canvas->timeOrigin())
                            + (hBar->value() + anchorViewportX) / m_canvas->pixelsPerNs();

    m_canvas->setPixelsPerNs(targetPpn);
    settleCanvasGeometry();

    // Re-derive the scroll position that puts the anchor instant back under the anchor
    // point. Both the applied scale (qBound may clamp the request) and the sliding-window
    // origin are re-read AFTER the rescale; the subtraction stays signed so an anchor that
    // slid out of the window clamps to the near edge instead of wrapping.
    const double newPpn = m_canvas->pixelsPerNs();
    const double delta = anchorTime - static_cast<double>(m_canvas->timeOrigin());
    hBar->setValue(qRound(delta * newPpn - anchorViewportX));
}

void LiveAnalyzerPanel::buttonZoomStep(double factor)
{
    if (m_followTail) {
        // Tracking the newest data: zoom around the tracked tail, scope-style — rescale
        // and re-pin so the newest edges hold the right side of the view and the follow
        // stays engaged, zooming in and out alike. (A center anchor here would slide the
        // tail off-screen and silently disengage the follow.)
        m_canvas->setPixelsPerNs(m_canvas->pixelsPerNs() * factor);
        settleCanvasGeometry();
        auto *hBar = m_scrollArea->horizontalScrollBar();
        hBar->setValue(hBar->maximum());
        return;
    }
    // Browsing history: magnify around the instant at the viewport center.
    applyAnchoredZoom(m_canvas->pixelsPerNs() * factor, m_scrollArea->viewport()->width() / 2.0);
}

void LiveAnalyzerPanel::zoomIn()
{
    buttonZoomStep(2.0);
}

void LiveAnalyzerPanel::zoomOut()
{
    buttonZoomStep(0.5);
}

void LiveAnalyzerPanel::zoomFit()
{
    const auto *rec = recorder();
    const SimTime maxTime = rec ? rec->maxTime() : 0;
    const int viewportWidth = m_scrollArea->viewport()->width();
    if (maxTime == 0 || viewportWidth <= 20) {
        return;
    }
    // The canvas pads its width by +20 px past the newest sample (sizeHint), so fitting
    // [0, maxTime] across the viewport means scaling to viewport − 20: the whole recording
    // plus the pad land exactly in view and the horizontal scroll range collapses to 0.
    m_canvas->setPixelsPerNs(static_cast<double>(viewportWidth - 20) / static_cast<double>(maxTime));
    settleCanvasGeometry();
    m_scrollArea->horizontalScrollBar()->setValue(0);

    // Fit shows the whole recording including the tail: re-engage the follow explicitly.
    // setValue(0) emits no valueChanged when the bar was already at 0, so the usual
    // recompute-on-scroll cannot be relied on here.
    m_followTail = true;
}

void LiveAnalyzerPanel::zoomToLatestEdge()
{
    const auto *rec = recorder();
    if (!rec) {
        return;
    }
    const SimTime latest = rec->maxTime();
    if (latest == 0) {
        emit statusMessage(tr("No recorded transitions yet."), 5000);
        return;
    }

    const int viewportWidth = m_scrollArea->viewport()->width();
    if (viewportWidth <= 0) {
        return;
    }

    // A delay-resolving window: a few hundred ns across the viewport puts a 5–20 ns gate
    // staircase at tens of pixels (setPixelsPerNs clamps to 10 px/ns for huge viewports).
    constexpr double kEdgeWindowNs = 250.0;
    m_canvas->setPixelsPerNs(viewportWidth / kEdgeWindowNs);
    settleCanvasGeometry();

    // Center the newest transition (the scroll bar clamps to its range: when the newest
    // edge sits at the canvas's right end — the usual case, since recording dedups — the
    // view pins near the tail with the edge cluster just left of the right margin). The
    // origin depends on the zoom, so read it after. x ≤ QWIDGETSIZE_MAX, no overflow.
    const SimTime origin = m_canvas->timeOrigin();
    const double x = (latest >= origin) ? static_cast<double>(latest - origin) * m_canvas->pixelsPerNs() : 0.0;
    auto *hBar = m_scrollArea->horizontalScrollBar();
    hBar->setValue(static_cast<int>(qMax(0.0, x - viewportWidth / 2.0)));

    if (m_scene && m_scene->simulation()->isRunning()) {
        emit statusMessage(tr("Pause the simulation to hold the view."), 5000);
    }
}

void LiveAnalyzerPanel::adaptZoomToTickWindow(SimTime nsPerTick)
{
    const double simNsPerWallSec = static_cast<double>(nsPerTick) * 1000.0; // ticks/sec = 1000 (1ms timer)
    const double visibleSimNs = simNsPerWallSec * 5.0;                      // 5 seconds of wall time
    const int availableWidth = m_scrollArea ? m_scrollArea->viewport()->width() : 0;
    if (availableWidth > 0 && visibleSimNs > 0) {
        m_canvas->setPixelsPerNs(static_cast<double>(availableWidth) / visibleSimNs);
    }
}

void LiveAnalyzerPanel::hintIfFunctionalMode()
{
    // Recording only advances a timeline on the temporal (propagation-delay) path; in
    // functional mode the diagram shows live levels at t = 0, so point the user at the
    // mode selector for actual waveforms.
    if (m_scene && m_scene->simulation()->timePerTick() == 0) {
        emit statusMessage(tr("Switch the simulation to Temporal mode to record waveforms."), 5000);
    }
}
