// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#if defined __cplusplus
// Standard library includes
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Qt includes -- wiredpanda_lib's own portable subset only (no Qt Widgets/PrintSupport/Test).
// The legacy Widgets-only wiredpanda/test_wiredpanda targets no longer reuse this PCH (see
// CMakeLists.txt) since their own sources still need the Widgets-family headers this file
// used to also carry; they compile without one now instead of needing a second, parallel PCH.

#include <QAudio>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QBitArray>
#include <QBrush>
#include <QBuffer>
#include <QByteArray>
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
#include <QPainter>
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
#include <QSizeF>
#include <QSoundEffect>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QStyleHints>
#include <QSvgGenerator>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTemporaryFile>
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

#endif
