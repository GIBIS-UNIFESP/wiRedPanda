// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickWorkSpace: CanvasItem-side port of App/Scene/Workspace.h's WorkSpace.
 */

#pragma once

#include <memory>

#include <QFileInfo>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QTimer>
#include <QVersionNumber>

class CanvasItem;
class QDataStream;

/**
 * \class QuickWorkSpace
 * \brief A single open circuit document -- the CanvasItem-side port of WorkSpace.
 *
 * \details Owns a CanvasItem instead of a GraphicsView + Scene pair; load()/save()/autosave()/
 * loadFromBlob() resolve through CanvasItem's public surface (deserializationContext()/
 * addItem()/icRegistry()/undoStack()/simulation()/setLastId()).
 *
 * Does not embed a minimap or ExerciseOverlay. Has no resizeEvent()/showEvent() analog:
 * CanvasItem's geometry is managed by whatever QML container parents it, not this class.
 */
class QuickWorkSpace : public QObject
{
    Q_OBJECT

    // Members (not free functions) so this grant covers direct access to private members
    // (m_loadedVersion/m_autosaveFileName/m_isInlineIC/m_fileInfo) for fixture setup. Mirrors
    // WorkSpace.h's own "friend class TestWorkspaceUnit;".
    friend class TestQuickWorkSpace;
    // Only ever reached through implicit property chains (AppController.currentTab.canvas()),
    // never spelled out as an explicit property type in any .qml file -- QML_ANONYMOUS
    // registers the type so qmllint can resolve currentTab's return type, without giving it a
    // QML-facing name to instantiate via Type {} syntax.
    QML_ANONYMOUS

public:
    /// Outcome of a save attempt. Mirrors WorkSpace::SaveOutcome.
    enum class SaveOutcome {
        Saved,
        ReadOnlyTarget,
    };

    explicit QuickWorkSpace(QObject *parent = nullptr);
    ~QuickWorkSpace() override;

    // --- Component Access ---

    /// Returns the canvas this workspace owns. Mirrors WorkSpace::view()/scene() combined --
    /// CanvasItem plays both roles (rendering + circuit/simulation state) on this side.
    /// Q_INVOKABLE so QML (the tab canvas host) can reach it directly off a QuickWorkSpace*.
    [[nodiscard]] Q_INVOKABLE CanvasItem *canvas() const { return m_canvas.get(); }

    // --- File Operations ---

    [[nodiscard]] QFileInfo fileInfo() const { return m_fileInfo; }
    void load(const QString &fileName);
    void load(QDataStream &stream, const QVersionNumber &version, const QString &contextDir);

    /// \details Same purity contract as WorkSpace::save(const QString&): \a fileName must
    /// already be a resolved, non-empty, ".panda"-suffixed path (inline-IC workspaces ignore
    /// it and serialize to a blob instead). Shows no dialogs itself and never recurses; a
    /// read-only/unwritable target is reported via the return value, not retried in place.
    SaveOutcome save(const QString &fileName);
    void save(QDataStream &stream);
    void setAutosaveFile();
    void flushPendingAutosave();

    // --- Inline IC Tab ---

    void loadFromBlob(const QByteArray &blob, QuickWorkSpace *parent, int icElementId, const QString &parentContextDir);

    [[nodiscard]] bool isInlineIC() const { return m_isInlineIC; }
    [[nodiscard]] QuickWorkSpace *parentWorkspace() const { return m_parentWorkspace; }
    [[nodiscard]] int parentICElementId() const { return m_parentICElementId; }
    [[nodiscard]] const QString &inlineBlobName() const { return m_inlineBlobName; }
    void setInlineBlobName(const QString &blobName) { m_inlineBlobName = blobName; }

    [[nodiscard]] const QString &untitledTitle() const { return m_untitledTitle; }
    void setUntitledTitle(const QString &title) { m_untitledTitle = title; }

    [[nodiscard]] bool isRecovered() const { return m_isRecovered; }
    void setRecovered(bool recovered) { m_isRecovered = recovered; }

    /// Removes all IC instances with the given blob name (and the blob itself), as one
    /// undoable macro. Mirrors WorkSpace::removeEmbeddedIC().
    void removeEmbeddedIC(const QString &blobName);

    // --- Waveform Integration ---

    [[nodiscard]] QString dolphinFileName() const { return m_dolphinFileName; }
    void setDolphinFileName(const QString &fileName) { m_dolphinFileName = fileName; }

    // --- ID Management ---

    [[nodiscard]] bool isFromNewerVersion() const;
    [[nodiscard]] int lastId() const { return m_lastId; }
    void setLastId(int newLastId) { m_lastId = newLastId; }

signals:
    /// Emitted whenever the file info of this workspace changes (load/save). Mirrors
    /// WorkSpace::fileChanged().
    void fileChanged(const QFileInfo &fileInfo);

    /// Emitted when an inline IC tab saves its blob (propagated to parent). Mirrors
    /// WorkSpace::icBlobSaved().
    void icBlobSaved(int icElementId, const QByteArray &blob);

public slots:
    /// Receives a saved blob from a child inline tab. Mirrors WorkSpace::onChildICBlobSaved().
    void onChildICBlobSaved(int icElementId, const QByteArray &blob);

private:
    void autosave();
    void setAutosaveFileName();

    /// Atomically sets m_fileInfo and the canvas's contextDir from \a filePath. Mirrors
    /// WorkSpace::setCurrentFile().
    void setCurrentFile(const QString &filePath);

    std::unique_ptr<CanvasItem> m_canvas;
    QFileInfo m_fileInfo;
    QString m_dolphinFileName;
    QString m_autosaveFileName;
    QTimer m_autosaveDebounceTimer;
    QVersionNumber m_loadedVersion;
    int m_lastId = 0;

    bool m_isInlineIC = false;
    QPointer<QuickWorkSpace> m_parentWorkspace;
    int m_parentICElementId = -1;
    QString m_inlineBlobName;
    QString m_untitledTitle;
    bool m_isRecovered = false;
};
