// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CircuitExporter: render a Scene to PDF or PNG image.
 */

#pragma once

class Scene;
class QString;

/**
 * \namespace CircuitExporter
 * \brief Pure render functions for exporting a circuit scene to image formats.
 *
 * \details Owns only the rendering step (QPrinter / QPixmap + QPainter).
 * File-dialog interaction and status-bar updates remain in MainWindow.
 * Both functions add a 64 px margin around the scene bounding rectangle.
 */
namespace CircuitExporter {

/**
 * \brief Renders \a scene to a PDF file at \a filePath.
 * \throws Pandaception if the QPainter cannot begin painting to the printer.
 */
void renderToPdf(Scene *scene, const QString &filePath);

/**
 * \brief Renders \a scene to a PNG image file at \a filePath.
 */
void renderToImage(Scene *scene, const QString &filePath);

} // namespace CircuitExporter
