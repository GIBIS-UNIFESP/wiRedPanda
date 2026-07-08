// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Discovery and translation lookup for Exercise/Tour step content.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

/// Metadata for one Exercise/Tour content JSON file, as discovered by scanning a directory
/// (built-in Qt resource, or a real filesystem folder). \a title and \a description are the
/// raw English-source strings straight from the JSON — NOT translated; callers apply
/// ExerciseTourResources::translate() themselves (mirrors
/// LanguageManager::availableLanguages() vs. displayName()).
struct ExerciseTourResourceEntry {
    QString path;          ///< ":/Exercises/x.json" (built-in) or a real filesystem path.
    QString id;            ///< JSON root "id" field.
    QString title;         ///< JSON root "title" field (raw, English-source).
    QString description;   ///< JSON root "description" field; "" if absent.
};

/**
 * \class ExerciseTourResources
 * \brief Static utility for discovering Exercise/Tour content and translating its text.
 *
 * \details Exercise/Tour JSON files are discovered by scanning directories (scan(), composed
 * by discover()) instead of a hardcoded list. Built-in content ships embedded at
 * ":/Exercises"/":/Tours"; on top of that, discover() also finds user-added content in three
 * real, non-built-in locations, each for a different audience:
 *  - the install-relative folder (InstallRelativePaths::candidates(), shared with the
 *    Examples menu) and, when that isn't writable, a Documents-based fallback — both
 *    end-user-facing, both opened by the "Open Folder" button in the browser dialogs via
 *    preferredContentDir();
 *  - the AppData folder (managedContentDir()) — reserved for a teacher/IT admin to
 *    pre-provision content in a managed classroom install. It is scanned so provisioned
 *    content shows up, but deliberately never opened or created by the "Open Folder" button
 *    or any other end-user-facing code path, so it stays out of ordinary users' way.
 *
 * Step text inside the JSON never passes through tr() (lupdate can't see JSON), so
 * translations instead live in a separate, Weblate-managed catalog at
 * :/i18n/ExerciseTour/<lang>.json; translate() looks strings up there, falling back to the
 * JSON's own English text when no translation is installed or found — which is also the
 * correct, automatic behavior for user-added content that will never have a catalog entry.
 */
class ExerciseTourResources
{
public:
    ExerciseTourResources() = delete;

    /// Scans \a directory (a Qt resource path like ":/Exercises", or a plain filesystem
    /// directory) for *.json files and returns one entry per well-formed file: a root object
    /// with non-empty "id" and "title". Files that fail to open, aren't valid JSON, aren't a
    /// JSON object, or are missing id/title are skipped (with a qWarning()); "description" is
    /// optional and defaults to "". Entries are sorted by id for deterministic UI ordering
    /// (QDir::entryList's order is not guaranteed).
    static QVector<ExerciseTourResourceEntry> scan(const QString &directory);

    /// Discovers all content for \a category ("Exercises" or "Tours"): the built-in bundled
    /// content at ":/<category>", the AppData folder (managedContentDir(), created if
    /// missing), the install-relative folder if one already exists (read-only check, creates
    /// nothing), and the Documents-based fallback folder if it happens to already exist
    /// (created only on demand by preferredContentDir(), never eagerly here). Entries whose id
    /// collides with one already found are skipped (qWarning()) rather than shown twice —
    /// earlier-scanned sources win, in this order: built-in, AppData folder, install-relative
    /// folder, Documents fallback.
    static QVector<ExerciseTourResourceEntry> discover(const QString &category);

    /// Returns the AppData folder for \a category (e.g. "Exercises"), creating it if it
    /// doesn't exist yet. Reserved exclusively for teacher/IT-provisioned content in a managed
    /// classroom install — scanned by discover(), but never opened or created by the "Open
    /// Folder" button or any other end-user-facing code path.
    static QString managedContentDir(const QString &category);

    /// Returns the folder the "Open Folder" button should open for \a category: the first
    /// install-relative candidate (InstallRelativePaths::candidates()) that already exists,
    /// or can be created, AND is writable — the simple, visible location meant for an
    /// individual end-user. If none of those are writable (e.g.
    /// installed under Program Files without admin rights), falls back to a Documents-based
    /// folder (QStandardPaths::DocumentsLocation + "/wiRedPanda/<category>"), created only at
    /// this point, on demand. Returns an empty string if even that fails — this never falls
    /// through to managedContentDir(), which must stay untouched by this path.
    static QString preferredContentDir(const QString &category);

    /// Merges \a additional into \a base: entries whose id already exists in \a base are
    /// skipped (qWarning()); the result is re-sorted by id. A public seam so the
    /// dedup/precedence policy is unit-testable without touching the filesystem or
    /// QStandardPaths.
    static QVector<ExerciseTourResourceEntry> mergeUnique(QVector<ExerciseTourResourceEntry> base,
                                                           const QVector<ExerciseTourResourceEntry> &additional);

    /// Looks up \a key (a dotted path, e.g. "basic-and-gate.title" or
    /// "basic-and-gate.place-and-gate.instruction") in the Weblate-managed catalog for
    /// Settings::language(). Returns \a fallbackEnglish immediately, without touching disk,
    /// when the language is empty or "en". Also returns \a fallbackEnglish if the catalog
    /// can't be opened/parsed or has no value at \a key.
    static QString translate(const QString &key, const QString &fallbackEnglish);

    /// Same dotted-path lookup as translate(), but reads \a catalogPath directly instead of
    /// deriving it from Settings::language(). translate() delegates to this; exposed
    /// separately so tests can exercise the parsing/fallback logic against a fixture file.
    static QString translateFromCatalog(const QString &catalogPath, const QString &key,
                                         const QString &fallbackEnglish);

    /// Tries each of \a candidates in order (create if missing, then check writable),
    /// returning the first that qualifies; falls back to \a documentsFallback (same
    /// create-then-check) if none do; returns "" if that also fails. preferredContentDir()
    /// delegates to this with real, environment-derived arguments; exposed separately, with
    /// no QStandardPaths/QCoreApplication dependency of its own, so tests can force the
    /// "nothing in the primary list is writable" branch without real OS permission
    /// manipulation.
    static QString resolveWritableDir(const QStringList &candidates, const QString &documentsFallback);

private:
    /// Computes (does not create) the Documents-based fallback path for \a category.
    static QString documentsFallbackDir(const QString &category);
};
