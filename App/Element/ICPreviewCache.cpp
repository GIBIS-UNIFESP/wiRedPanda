// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICPreviewCache.h"

#include <list>

#include <QCoreApplication>
#include <QObject>
#include <QThread>

#include "App/Core/ThemeManager.h"
#include "App/Element/ICDefinition.h"
#include "App/Element/ICLoader.h"

namespace {

/// One preview kept: the definition it was rendered from (through a weak reference), the directory, and the picture.
struct PreviewEntry {
    std::weak_ptr<const ICDefinition> definition;
    QString contextDir;
    QPixmap pixmap;
};

/// The cache's state, on the main thread only.
struct PreviewCache {
    std::list<PreviewEntry> entries; ///< Most recently used first.
    int renders = 0;
};

/// Returns the cache. It is never destroyed: an IC can outlive the statics.
PreviewCache &previewCache()
{
    static PreviewCache *instance = new PreviewCache;
    return *instance;
}

/// Connects the cache to the theme, once, on first use: a preview is drawn in the theme's colours.
void dropOnThemeChange()
{
    static const bool connected = [] {
        QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, [] { ICPreviewCache::clear(); });
        return true;
    }();
    Q_UNUSED(connected)
}

} // namespace

QPixmap ICPreviewCache::preview(const std::shared_ptr<const ICDefinition> &definition, const QString &contextDir)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());
    auto &c = previewCache();

    // A definition that is gone (its IC reloaded or closed) takes its preview with it
    c.entries.remove_if([](const PreviewEntry &entry) { return entry.definition.expired(); });

    for (auto found = c.entries.begin(); found != c.entries.end(); ++found) {
        if (found->definition.lock() == definition && found->contextDir == contextDir) {
            c.entries.splice(c.entries.begin(), c.entries, found);
            return c.entries.front().pixmap;
        }
    }

    ++c.renders;
    c.entries.push_front(PreviewEntry{definition, contextDir, ICLoader::renderPreview(*definition, contextDir)});
    if (c.entries.size() > kCapacity) {
        c.entries.pop_back();
    }
    dropOnThemeChange();
    return c.entries.front().pixmap;
}

void ICPreviewCache::clear()
{
    previewCache().entries.clear();
}

int ICPreviewCache::renderCount()
{
    return previewCache().renders;
}

int ICPreviewCache::entryCount()
{
    return static_cast<int>(previewCache().entries.size());
}
