// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/CircuitExporter.h"

#include <algorithm>

#include <QImage>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>
#include <QRectF>
#include <QSizeF>

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

QImage renderScaledImage(Scene *scene, const QRectF &paddedRect)
{
    // Cap the output at a sane maximum dimension. paddedRect derives from element positions
    // loaded from a .panda file (or set via the MCP create_element/move_element commands, which
    // accept any finite coordinate); the only check on load (GraphicElementSerializer's position
    // validation) rejects non-finite coordinates but never bounds magnitude, so a crafted or
    // corrupted file — or a scripted MCP client — with one element at a large-but-finite
    // position would otherwise size this image proportionally — potentially tens of gigabytes.
    // Scale into the capped size instead of failing outright, the same "fit scene content to a
    // fixed target" technique renderToPdf already uses via its fixed-size printer page. Below
    // the cap (every realistic circuit), output resolution is unchanged.
    QSizeF targetSize = paddedRect.size();
    const qreal scale = (std::min)({1.0, kMaxImageDimension / targetSize.width(), kMaxImageDimension / targetSize.height()});
    if (scale < 1.0) {
        targetSize *= scale;
    }

    // QImage with an explicit alpha format, not QPixmap — QPixmap(size) defaults to an
    // opaque platform format on this build, so filling it with Qt::transparent silently
    // flattens to opaque instead of storing real alpha. Without an explicit fill at all it's
    // uninitialized (Qt's documented QPixmap/QImage(size) contract): Scene only paints its
    // background opaquely once Scene::updateTheme() has run, which production always does
    // first, masking this everywhere else; a caller that doesn't would export genuine
    // garbage/uninitialized pixels in the padding margin.
    QImage image(targetSize.toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter;
    if (!painter.begin(&image)) {
        throw PANDACEPTION_WITH_CONTEXT("CircuitExporter", "Could not begin painting circuit image.");
    }
    // Antialiasing enabled here; the PDF path relies on the printer's high DPI instead.
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(QPointF(), targetSize), paddedRect);
    painter.end();

    return image;
}

void renderToImage(Scene *scene, const QString &filePath)
{
    const QImage image = renderScaledImage(scene, paddedBoundingRect(scene));

    if (!image.save(filePath)) {
        throw PANDACEPTION_WITH_CONTEXT("CircuitExporter", "Could not save image to %1.", filePath);
    }
}

} // namespace CircuitExporter
