// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/WaveformGrid.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>

static const QColor kGreen(0x00, 0x80, 0x00);
static const QColor kBlue(0x75, 0x8E, 0xFF);

WaveformGrid::WaveformGrid(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setFlag(ItemIsFocusable, true);
    setFocus();
}

// --- Data setup ---

void WaveformGrid::load(const QStringList &inputLabels, const QStringList &outputLabels, const int length)
{
    prepareGeometryChange();
    m_rows.clear();
    m_inputRowCount = static_cast<int>(inputLabels.size());
    m_length = length;

    for (const auto &label : inputLabels) {
        m_rows.append({label, true, QVector<int>(length, 0)});
    }

    for (const auto &label : outputLabels) {
        m_rows.append({label, false, QVector<int>(length, 0)});
    }

    m_cursorRow = 0;
    m_cursorCol = 0;
    m_anchorRow = 0;
    m_anchorCol = 0;
    update();
}

void WaveformGrid::setLength(const int newLength)
{
    if (newLength == m_length) {
        return;
    }

    prepareGeometryChange();

    for (auto &row : m_rows) {
        row.values.resize(newLength);

        for (int col = m_length; col < newLength; ++col) {
            row.values[col] = 0;
        }
    }

    m_length = newLength;

    if (m_cursorCol >= m_length) {
        m_cursorCol = m_length - 1;
    }

    if (m_anchorCol >= m_length) {
        m_anchorCol = m_length - 1;
    }

    update();
}

// --- Cell access ---

void WaveformGrid::setCellValue(const int row, const int col, const int value)
{
    if (row < 0 || row >= m_rows.size() || col < 0 || col >= m_length) {
        return;
    }

    m_rows[row].values[col] = value;
    update();
}

int WaveformGrid::cellValue(const int row, const int col) const
{
    if (row < 0 || row >= m_rows.size() || col < 0 || col >= m_length) {
        return 0;
    }

    return m_rows[row].values[col];
}

QString WaveformGrid::rowLabel(const int row) const
{
    if (row < 0 || row >= m_rows.size()) {
        return {};
    }

    return m_rows[row].label;
}

// --- Display mode ---

void WaveformGrid::setPlotType(const PlotType type)
{
    m_plotType = type;
    update();
}

// --- Selection ---

QRect WaveformGrid::selectionRect() const
{
    const int r1 = std::min(m_anchorRow, m_cursorRow);
    const int r2 = std::max(m_anchorRow, m_cursorRow);
    const int c1 = std::min(m_anchorCol, m_cursorCol);
    const int c2 = std::max(m_anchorCol, m_cursorCol);
    return {c1, r1, c2 - c1 + 1, r2 - r1 + 1};
}

QVector<QPair<int, int>> WaveformGrid::selectedCells() const
{
    QVector<QPair<int, int>> cells;
    const QRect sel = selectionRect();

    for (int r = sel.y(); r < sel.y() + sel.height(); ++r) {
        for (int c = sel.x(); c < sel.x() + sel.width(); ++c) {
            cells.append({r, c});
        }
    }

    return cells;
}

bool WaveformGrid::hasSelection() const
{
    return !m_rows.isEmpty() && m_length > 0;
}

int WaveformGrid::selectionFirstRow() const
{
    return std::min(m_anchorRow, m_cursorRow);
}

int WaveformGrid::selectionFirstCol() const
{
    return std::min(m_anchorCol, m_cursorCol);
}

void WaveformGrid::selectAll()
{
    m_anchorRow = 0;
    m_anchorCol = 0;
    m_cursorRow = rowCount() - 1;
    m_cursorCol = m_length - 1;
    update();
    emit selectionChanged();
}

void WaveformGrid::clearSelection()
{
    m_anchorRow = m_cursorRow;
    m_anchorCol = m_cursorCol;
    update();
    emit selectionChanged();
}

// --- Cursor ---

void WaveformGrid::setCursorPos(const int row, const int col)
{
    m_cursorRow = std::clamp(row, 0, std::max(0, rowCount() - 1));
    m_cursorCol = std::clamp(col, 0, std::max(0, m_length - 1));
    m_anchorRow = m_cursorRow;
    m_anchorCol = m_cursorCol;
    update();
    emit selectionChanged();
}

void WaveformGrid::moveCursor(const int dRow, const int dCol, const bool extendSelection)
{
    if (m_rows.isEmpty() || m_length == 0) {
        return;
    }

    m_cursorRow = std::clamp(m_cursorRow + dRow, 0, rowCount() - 1);
    m_cursorCol = std::clamp(m_cursorCol + dCol, 0, m_length - 1);

    if (!extendSelection) {
        m_anchorRow = m_cursorRow;
        m_anchorCol = m_cursorCol;
    }

    update();
    emit selectionChanged();
}

void WaveformGrid::toggleAtCursor()
{
    if (!isInputRow(m_cursorRow)) {
        return;
    }

    const int old = cellValue(m_cursorRow, m_cursorCol);
    const int val = (old + 1) % 2;
    setCellValue(m_cursorRow, m_cursorCol, val);
    emit cellEdited(m_cursorRow, m_cursorCol, val);
}

// --- QGraphicsItem ---

QRectF WaveformGrid::boundingRect() const
{
    const qreal w = kHeaderW + static_cast<qreal>(m_length) * kCellW;
    const qreal h = kHeaderH + static_cast<qreal>(rowCount()) * kCellH;
    return {0, 0, w, h};
}

// --- Hit testing ---

bool WaveformGrid::cellAt(const QPointF &pos, int &row, int &col) const
{
    const qreal x = pos.x() - kHeaderW;
    const qreal y = pos.y() - kHeaderH;

    if (x < 0 || y < 0) {
        return false;
    }

    col = static_cast<int>(x / kCellW);
    row = static_cast<int>(y / kCellH);

    return (row >= 0 && row < rowCount() && col >= 0 && col < m_length);
}

QRectF WaveformGrid::cellRect(const int row, const int col) const
{
    return {
        kHeaderW + static_cast<qreal>(col) * kCellW,
        kHeaderH + static_cast<qreal>(row) * kCellH,
        static_cast<qreal>(kCellW),
        static_cast<qreal>(kCellH)
    };
}

// --- Paint ---

void WaveformGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    const QPalette pal = QApplication::palette();
    painter->setRenderHint(QPainter::Antialiasing, false);

    // Background
    painter->fillRect(boundingRect(), pal.base());

    // Row backgrounds (alternating)
    for (int r = 0; r < rowCount(); ++r) {
        paintRowBackground(painter, r, pal);
    }

    // Column header
    paintColHeader(painter, pal);

    // Row labels and signal data
    for (int r = 0; r < rowCount(); ++r) {
        paintRowLabel(painter, r, pal);

        if (m_plotType == PlotType::Line) {
            paintWaveformRow(painter, r);
        } else {
            paintNumberRow(painter, r, pal);
        }
    }

    // Selection overlay and cursor
    paintSelectionOverlay(painter, pal);
    paintCursor(painter, pal);
}

void WaveformGrid::paintColHeader(QPainter *p, const QPalette &pal) const
{
    p->fillRect(QRectF(0, 0, boundingRect().width(), kHeaderH), pal.button());

    p->setPen(pal.buttonText().color());
    QFont font = p->font();
    font.setPixelSize(11);
    p->setFont(font);

    for (int col = 0; col < m_length; ++col) {
        const QRectF rect(kHeaderW + col * kCellW, 0, kCellW, kHeaderH);
        p->drawText(rect, Qt::AlignCenter, QString::number(col));
    }

    p->setPen(QPen(pal.mid().color(), 0));
    p->drawLine(QPointF(0, kHeaderH), QPointF(boundingRect().width(), kHeaderH));
}

void WaveformGrid::paintRowBackground(QPainter *p, const int row, const QPalette &pal) const
{
    if (row % 2 == 1) {
        const QRectF rect(0, kHeaderH + row * kCellH, boundingRect().width(), kCellH);
        p->fillRect(rect, pal.alternateBase());
    }
}

void WaveformGrid::paintRowLabel(QPainter *p, const int row, const QPalette &pal) const
{
    const QRectF rect(0, kHeaderH + row * kCellH, kHeaderW - 4, kCellH);

    p->setPen(pal.text().color());
    QFont font = p->font();
    font.setPixelSize(12);
    p->setFont(font);
    p->drawText(rect, Qt::AlignVCenter | Qt::AlignRight, m_rows[row].label);

    p->setPen(QPen(pal.mid().color(), 0));
    p->drawLine(QPointF(kHeaderW, kHeaderH + row * kCellH),
                QPointF(kHeaderW, kHeaderH + (row + 1) * kCellH));
}

void WaveformGrid::paintWaveformRow(QPainter *p, const int row) const
{
    const auto &signal = m_rows[row];

    for (int col = 0; col < m_length; ++col) {
        const QRectF rect = cellRect(row, col);
        const int value = signal.values[col];
        const int prevValue = (col > 0) ? signal.values[col - 1] : 0;
        const bool hasPrev = (col > 0);
        paintWaveformCell(p, rect, value, prevValue, hasPrev, signal.isInput);
    }
}

void WaveformGrid::paintNumberRow(QPainter *p, const int row, const QPalette &pal) const
{
    const auto &signal = m_rows[row];

    p->setPen(pal.text().color());
    QFont font = p->font();
    font.setPixelSize(12);
    p->setFont(font);

    for (int col = 0; col < m_length; ++col) {
        const QRectF rect = cellRect(row, col);
        p->drawText(rect, Qt::AlignCenter, QString::number(signal.values[col]));
    }
}

void WaveformGrid::paintWaveformCell(QPainter *p, const QRectF &rect,
                                      const int value, const int prevValue,
                                      const bool hasPrev, const bool isInput) const
{
    const QColor &color = isInput ? kBlue : kGreen;
    const bool isHigh  = (value == 1);
    const bool hasEdge = hasPrev && (prevValue != value);

    const qreal H = rect.height();
    const qreal W = rect.width();
    const qreal x = rect.x();
    const qreal y = rect.y();

    // Shadow fill (half-opacity)
    QColor shadow(color);
    shadow.setAlphaF(0.5);

    if (isHigh) {
        p->fillRect(QRectF(x, y + 12.0 / 30.0 * H, W, 18.0 / 30.0 * H), shadow);
    } else if (hasEdge) {
        p->fillRect(QRectF(x, y + 22.0 / 30.0 * H, W, 8.0 / 30.0 * H), shadow);
    } else {
        p->fillRect(QRectF(x, y + 24.0 / 30.0 * H, W, 6.0 / 30.0 * H), shadow);
    }

    // Horizontal signal line
    const qreal lineY = (isHigh ? 8.0 : 20.0) / 30.0 * H;
    p->fillRect(QRectF(x, y + lineY, W, 4.0 / 30.0 * H), color);

    // Vertical transition bar
    if (hasEdge) {
        const qreal barW = 4.0 / 100.0 * W;
        p->fillRect(QRectF(x, y + 8.0 / 30.0 * H, barW, 16.0 / 30.0 * H), color);
    }
}

void WaveformGrid::paintSelectionOverlay(QPainter *p, const QPalette &pal) const
{
    const QRect sel = selectionRect();

    if (sel.width() <= 0 || sel.height() <= 0) {
        return;
    }

    const qreal x = kHeaderW + sel.x() * kCellW;
    const qreal y = kHeaderH + sel.y() * kCellH;
    const qreal w = sel.width() * kCellW;
    const qreal h = sel.height() * kCellH;

    QColor highlight = pal.highlight().color();
    highlight.setAlpha(80);
    p->fillRect(QRectF(x, y, w, h), highlight);
}

void WaveformGrid::paintCursor(QPainter *p, const QPalette &pal) const
{
    if (!m_hasFocus || m_rows.isEmpty() || m_length == 0) {
        return;
    }

    const QRectF rect = cellRect(m_cursorRow, m_cursorCol);
    p->setPen(QPen(pal.text().color(), 2));
    p->setBrush(Qt::NoBrush);
    p->drawRect(rect.adjusted(1, 1, -1, -1));
}

// --- Mouse events ---

void WaveformGrid::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setFocus();
    int row, col;

    if (cellAt(event->pos(), row, col)) {
        const bool extend = (event->modifiers() & Qt::ShiftModifier);

        if (extend) {
            m_cursorRow = row;
            m_cursorCol = col;
        } else {
            m_cursorRow = row;
            m_cursorCol = col;
            m_anchorRow = row;
            m_anchorCol = col;
        }

        update();
        emit selectionChanged();
    }

    event->accept();
}

void WaveformGrid::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int row, col;

    if (cellAt(event->pos(), row, col)) {
        m_cursorRow = row;
        m_cursorCol = col;
        update();
        emit selectionChanged();
    }

    event->accept();
}

void WaveformGrid::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    int row, col;

    if (cellAt(event->pos(), row, col) && isInputRow(row)) {
        const int old = cellValue(row, col);
        const int val = (old + 1) % 2;
        setCellValue(row, col, val);
        emit cellEdited(row, col, val);
    }

    event->accept();
}

// --- Keyboard events ---

void WaveformGrid::keyPressEvent(QKeyEvent *event)
{
    const bool shift = (event->modifiers() & Qt::ShiftModifier);

    switch (event->key()) {
    case Qt::Key_Left:  moveCursor(0, -1, shift); break;
    case Qt::Key_Right: moveCursor(0, +1, shift); break;
    case Qt::Key_Up:    moveCursor(-1, 0, shift); break;
    case Qt::Key_Down:  moveCursor(+1, 0, shift); break;
    case Qt::Key_Space: toggleAtCursor(); break;
    default:
        QGraphicsObject::keyPressEvent(event);
        return;
    }

    event->accept();
}

void WaveformGrid::focusInEvent(QFocusEvent *event)
{
    m_hasFocus = true;
    update();
    QGraphicsObject::focusInEvent(event);
}

void WaveformGrid::focusOutEvent(QFocusEvent *event)
{
    m_hasFocus = false;
    update();
    QGraphicsObject::focusOutEvent(event);
}
