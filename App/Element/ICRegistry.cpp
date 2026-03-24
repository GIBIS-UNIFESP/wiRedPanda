// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICRegistry.h"

#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"

ICRegistry::ICRegistry(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ICRegistry::onFileChanged);
}

bool ICRegistry::has(const QString &filePath) const
{
    return m_definitions.contains(filePath);
}

const ICDefinition *ICRegistry::definition(const QString &filePath, const QString &contextDir)
{
    if (!m_definitions.contains(filePath)) {
        m_definitions[filePath] = ICDefinition::fromFile(filePath, contextDir);
    }
    return &m_definitions[filePath];
}

void ICRegistry::invalidate(const QString &filePath)
{
    m_definitions.remove(filePath);
}

void ICRegistry::watchFile(const QString &filePath)
{
    if (!m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

QList<GraphicElement *> ICRegistry::findICsByFile(const QString &fileName) const
{
    QList<GraphicElement *> result;
    for (auto *elm : m_scene->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            if (QFileInfo(ic->icFile()) == QFileInfo(fileName)) {
                result.append(elm);
            }
        }
    }
    return result;
}

bool ICRegistry::isLoading(const QString &canonicalPath) const
{
    return m_loadingFiles.contains(canonicalPath);
}

bool ICRegistry::beginLoading(const QString &canonicalPath)
{
    if (m_loadingFiles.contains(canonicalPath)) {
        return false;
    }
    m_loadingFiles.insert(canonicalPath);
    return true;
}

void ICRegistry::endLoading(const QString &canonicalPath)
{
    m_loadingFiles.remove(canonicalPath);
}

void ICRegistry::onFileChanged(const QString &filePath)
{
    qCDebug(zero) << "IC file changed:" << filePath;

    // Invalidate the cached definition so it's rebuilt on next access
    invalidate(filePath);

    // Re-add the watch (some OS remove it after a file change event)
    if (!m_fileWatcher.files().contains(filePath) && QFileInfo::exists(filePath)) {
        m_fileWatcher.addPath(filePath);
    }

    // Reload all IC instances referencing this file
    const auto targets = findICsByFile(filePath);
    for (auto *elm : targets) {
        auto *ic = static_cast<IC *>(elm);
        ic->loadFile(filePath, QFileInfo(filePath).absolutePath());
    }

    if (!targets.isEmpty()) {
        m_scene->simulation()->restart();
    }

    emit definitionChanged(filePath);
}

