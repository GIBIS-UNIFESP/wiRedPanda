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
 * \details Copy-and-adapt port (same precedent as CanvasCommands/CanvasICRegistry), not a
 * modification of the production WorkSpace: owns a CanvasItem instead of a GraphicsView +
 * Scene pair. load()/save()/autosave()/loadFromBlob() resolve almost entirely through
 * CanvasItem's Phase-3 surface (deserializationContext()/addItem()/icRegistry()/
 * undoStack()/simulation()/setLastId()) -- see the plan's "Phase 4 in depth" section for the
 * finding that made this port straightforward instead of another foundational build-out.
 *
 * Deliberately NOT ported here (real, named deferrals, not oversights): the minimap
 * (WorkSpace embeds MinimapWidget directly in its constructor and manages its geometry in
 * resizeEvent()/showEvent() -- both are Phase 4 sub-step 7's job, a real widget rewrite, not
 * wiring), and ExerciseOverlay integration (Phase 5, once ExerciseOverlay itself is rewritten
 * for Quick). resizeEvent()/showEvent() themselves have no analog here at all: CanvasItem's
 * own geometry is managed by whatever QML container parents it, not this class's concern the
 * way WorkSpace (a QWidget) had to manage its child widgets' layout directly.
 */
class QuickWorkSpace : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(CanvasItem *canvas READ canvas CONSTANT FINAL)

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
    /// A plain Q_PROPERTY (not Q_INVOKABLE): the pointer is fixed at construction and
    /// never reassigned, and being a property (not a method call) lets QML's shadow
    /// check resolve chains through it instead of treating every further lookup as
    /// unresolvable "var" -- see project_qml_aot_compilation_fusion_style_pin memory.
    [[nodiscard]] CanvasItem *canvas() const { return m_canvas.get(); }

    // --- File Operations ---

    /// Display title for this tab (file name, numbered placeholder, or "[blob]" for an
    /// inline IC tab), without the unsaved "*" marker. A real Q_PROPERTY (not a plain
    /// invokable) so the tab bar's binding actually stays live -- titleChanged() is
    /// emitted alongside every fileChanged() this class already fires.
    [[nodiscard]] QString title() const;
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

    /// Emitted whenever title() may have changed -- paired with every fileChanged() emit.
    void titleChanged();

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
