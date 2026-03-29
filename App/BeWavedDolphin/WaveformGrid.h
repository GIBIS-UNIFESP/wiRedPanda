// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QGraphicsObject>
#include <QPalette>
#include <QVector>

/// Controls how signal cells are rendered in the waveform table.
enum class PlotType {
    Number, ///< Cells display the numeric value (0/1).
    Line    ///< Cells display a waveform-style rising/falling edge graphic.
};

class WaveformGrid : public QGraphicsObject
{
    Q_OBJECT

public:
    static constexpr int kHeaderW = 120;
    static constexpr int kHeaderH = 25;
    static constexpr int kCellW   = 49;
    static constexpr int kCellH   = 38;

    explicit WaveformGrid(QGraphicsItem *parent = nullptr);

    // --- Data setup ---

    void load(const QStringList &inputLabels, const QStringList &outputLabels, int length);
    void setLength(int newLength);
    int  length()        const { return m_length; }
    int  rowCount()      const { return static_cast<int>(m_rows.size()); }
    int  inputRowCount() const { return m_inputRowCount; }

    // --- Cell access ---

    void setCellValue(int row, int col, int value);
    int  cellValue(int row, int col) const;
    bool isInputRow(int row) const { return row < m_inputRowCount; }
    QString rowLabel(int row) const;

    // --- Display mode ---

    void setPlotType(PlotType type);
    PlotType plotType() const { return m_plotType; }

    // --- Selection ---

    QVector<QPair<int, int>> selectedCells() const;
    bool hasSelection() const;
    int  selectionFirstRow() const;
    int  selectionFirstCol() const;
    void selectAll();
    void clearSelection();

    // --- Cursor ---

    int  cursorRow() const { return m_cursorRow; }
    int  cursorCol() const { return m_cursorCol; }
    void setCursorPos(int row, int col);
    void moveCursor(int dRow, int dCol, bool extendSelection = false);
    void toggleAtCursor();

    // --- QGraphicsItem ---

    QRectF boundingRect() const override;

signals:
    void selectionChanged();
    void cellEdited(int row, int col, int newValue);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    struct SignalRow {
        QString label;
        bool isInput = false;
        QVector<int> values;
    };

    // --- Paint helpers ---

    void paintColHeader(QPainter *p, const QPalette &pal) const;
    void paintRowBackground(QPainter *p, int row, const QPalette &pal) const;
    void paintRowLabel(QPainter *p, int row, const QPalette &pal) const;
    void paintWaveformRow(QPainter *p, int row) const;
    void paintNumberRow(QPainter *p, int row, const QPalette &pal) const;
    void paintWaveformCell(QPainter *p, const QRectF &rect, int value, int prevValue, bool hasPrev, bool isInput) const;
    void paintSelectionOverlay(QPainter *p, const QPalette &pal) const;
    void paintCursor(QPainter *p, const QPalette &pal) const;

    bool cellAt(const QPointF &pos, int &row, int &col) const;
    QRectF cellRect(int row, int col) const;
    QRect selectionRect() const;

    // --- Data ---

    QVector<SignalRow> m_rows;
    int m_length       = 0;
    int m_inputRowCount = 0;
    PlotType m_plotType = PlotType::Line;

    // --- Selection state ---

    int m_cursorRow = 0;
    int m_cursorCol = 0;
    int m_anchorRow = 0;
    int m_anchorCol = 0;
    bool m_hasFocus = false;
};
