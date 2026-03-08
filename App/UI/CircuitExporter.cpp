// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/CircuitExporter.h"

#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QRectF>

#include "App/Core/Common.h"
#include "App/Scene/Scene.h"

namespace CircuitExporter {

// Add a 64 px margin so elements at the scene boundary are not clipped.
static QRectF paddedBoundingRect(Scene *scene)
{
    return scene->itemsBoundingRect().adjusted(-64, -64, 64, 64);
}

void renderToPdf(Scene *scene, const QString &filePath)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    // Landscape fits most circuits better than portrait.
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);

    QPainter painter;

    if (!painter.begin(&printer)) {
        throw PANDACEPTION_WITH_CONTEXT("CircuitExporter", "Could not print this circuit to PDF.");
    }

    scene->render(&painter, QRectF(), paddedBoundingRect(scene));
    painter.end();
}

void renderToImage(Scene *scene, const QString &filePath)
{
    const QRectF rect = paddedBoundingRect(scene);
    QPixmap pixmap(rect.size().toSize());

    QPainter painter;
    painter.begin(&pixmap);
    // Antialiasing enabled here; the PDF path relies on the printer's high DPI instead.
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(), rect);
    painter.end();

    pixmap.save(filePath);
}

} // namespace CircuitExporter
