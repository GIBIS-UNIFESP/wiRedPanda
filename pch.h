// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#if defined __cplusplus
// Standard library includes
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Qt includes
//
// Deliberately Qt6::Widgets/PrintSupport-free: this PCH backs wiredpanda_lib, whose compiled
// sources (see CMakeSources.cmake's SOURCES list, split for Quick in Task 106) have no real
// Widgets dependency. wiredpanda/test_wiredpanda (the Widgets-only targets) REUSE_FROM this
// PCH too, but that's a compile-speed optimization only -- any Widgets header their own
// sources need gets pulled in normally by their #includes, same as any header not listed here.

#include <QAction>
#include <QActionGroup>
#include <QAudio>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QBitArray>
#include <QBrush>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCursor>
#include <QDataStream>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QElapsedTimer>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFontMetrics>
#include <QHash>
#include <QIODevice>
#include <QIcon>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QKeyEvent>
#include <QKeySequence>
#include <QList>
#include <QLocale>
#include <QLoggingCategory>
#include <QMap>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QMetaEnum>
#include <QMimeData>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QPixmapCache>
#include <QPoint>
#include <QPointF>
#include <QPointer>
#include <QProcess>
#include <QRandomGenerator>
#include <QRectF>
#include <QRegularExpression>
#include <QResource>
#include <QSaveFile>
#include <QScopeGuard>
#include <QSet>
#include <QSettings>
#include <QShortcut>
#include <QSignalSpy>
#include <QSizeF>
#include <QSoundEffect>
#include <QStack>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QStyleHints>
#include <QSvgGenerator>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTextStream>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QTranslator>
#include <QUndoCommand>
#include <QUndoStack>
#include <QUrl>
#include <QUuid>
#include <QVariant>
#include <QVector>
#include <QVersionNumber>
#include <QWheelEvent>
#include <QtGlobal>
#include <QtTest>

#endif
