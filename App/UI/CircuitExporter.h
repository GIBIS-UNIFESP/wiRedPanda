// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CircuitExporter: render a Scene to PDF or PNG image.
 */

#pragma once

class Scene;
class QString;
class QImage;
class QRectF;

/**
 * \namespace CircuitExporter
 * \brief Pure render functions for exporting a circuit scene to image formats.
 *
 * \details Owns only the rendering step (QPrinter / QPixmap + QPainter).
 * File-dialog interaction and status-bar updates remain in MainWindow.
 * Both functions add a 64 px margin around the scene bounding rectangle.
 */
namespace CircuitExporter {

/// Maximum width/height (in pixels) renderToImage() will allocate. Scene extent beyond this
/// is scaled down to fit rather than sized proportionally — see renderToImage()'s details.
/// Bounded well under 8192 (where width*height*4 bytes hits Qt's own default 256 MB
/// QImageIOHandler allocation limit — confirmed via debugger inspection: a 16384 cap produced
/// an image QImage's own reader then refused to load back), so the exported file stays usable
/// by any default-configured Qt image consumer, not just by this process's writer.
inline constexpr double kMaxImageDimension = 4096;

/**
 * \brief Renders \a scene to a PDF file at \a filePath.
 * \throws Pandaception if the QPainter cannot begin painting to the printer.
 */
void renderToPdf(Scene *scene, const QString &filePath);

/**
 * \brief Renders \a scene to a PNG image file at \a filePath.
 * \details Output resolution matches the scene extent 1:1 up to kMaxImageDimension;
 * larger scenes are scaled down to fit rather than allocating proportionally.
 */
void renderToImage(Scene *scene, const QString &filePath);

/**
 * \brief Renders \a scene into a transparent-filled QImage bounded by kMaxImageDimension.
 * \details \a paddedRect is scene-coordinate content to render (caller applies its own
 * padding policy). Content is scaled down to fit within kMaxImageDimension per side rather
 * than allocated proportionally to \a paddedRect. Shared by renderToImage() and the MCP
 * export_image handler so this bounding logic has exactly one implementation.
 * \throws Pandaception if the QPainter cannot begin painting to the image.
 */
QImage renderScaledImage(Scene *scene, const QRectF &paddedRect);

} // namespace CircuitExporter
