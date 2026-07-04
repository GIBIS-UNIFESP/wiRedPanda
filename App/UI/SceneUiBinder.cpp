// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/SceneUiBinder.h"

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QShortcut>
#include <QStatusBar>
#include <QUndoStack>

#include "App/Core/Common.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/UI/ElementEditor.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/MainWindowUI.h"

SceneUiBinder::SceneUiBinder(MainWindowUi *ui, ElementPalette *palette, ICPreviewPopup *previewPopup, QWidget *shortcutParent, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
    , m_palette(palette)
    , m_previewPopup(previewPopup)
{
    // [ / ] cycle a selected element's primary property (e.g. input size).
    // { / } cycle a secondary property; < / > morph through element variants.
    // They are connected to the active scene in bind() so they follow the current tab.
    m_prevMainPropShortcut  = new QShortcut(QKeySequence("["), shortcutParent);
    m_nextMainPropShortcut  = new QShortcut(QKeySequence("]"), shortcutParent);
    m_prevSecndPropShortcut = new QShortcut(QKeySequence("{"), shortcutParent);
    m_nextSecndPropShortcut = new QShortcut(QKeySequence("}"), shortcutParent);
    m_changePrevElmShortcut = new QShortcut(QKeySequence("<"), shortcutParent);
    m_changeNextElmShortcut = new QShortcut(QKeySequence(">"), shortcutParent);
}

void SceneUiBinder::addUndoRedoMenu()
{
    if (!m_bound) {
        return;
    }

    auto *scene = m_bound->scene();
    if (!scene) {
        return;
    }

    const auto actions = m_ui->menuEdit->actions();
    if (actions.size() < 2) {
        return;
    }

    // Insert before position 0 then before the new position 1 so undo appears
    // first, redo second — above the separator that already exists in menuEdit.
    m_ui->menuEdit->insertAction(actions.at(0), scene->undoAction());
    m_ui->menuEdit->insertAction(m_ui->menuEdit->actions().at(1), scene->redoAction());
}

void SceneUiBinder::removeUndoRedoMenu()
{
    if (!m_bound) {
        return;
    }

    // The undo/redo actions are always inserted at positions 0 and 1 of menuEdit.
    // Any fewer entries means they were never added, so nothing to remove.
    const auto actions = m_ui->menuEdit->actions();
    if (actions.size() < 2) {
        return;
    }
    m_ui->menuEdit->removeAction(actions.at(0));
    m_ui->menuEdit->removeAction(actions.at(1));
}

void SceneUiBinder::syncZoomActions()
{
    if (!m_bound) {
        return;
    }

    m_ui->actionZoomIn->setEnabled(m_bound->view()->canZoomIn());
    m_ui->actionZoomOut->setEnabled(m_bound->view()->canZoomOut());
}

void SceneUiBinder::bind(WorkSpace *tab)
{
    m_bound = tab;
    auto *scene = tab->scene();

    qCDebug(zero) << "Connecting undo and redo functions to UI menu.";
    addUndoRedoMenu();

    m_palette->updateEmbeddedICList(scene);

    qCDebug(zero) << "Connecting current tab to element editor menu in UI.";
    m_ui->elementEditor->setScene(scene);

    connect(tab->view(),       &GraphicsView::zoomChanged,     this, &SceneUiBinder::syncZoomActions);
    connect(tab->simulation(), &Simulation::simulationWarning, this, [this](const QString &msg) {
        m_ui->statusBar->showMessage(msg, 8000);
    });
    connect(scene->undoStack(), &QUndoStack::indexChanged, this, [this] {
        if (m_bound) {
            m_palette->updateEmbeddedICList(m_bound->scene());
        }
    });

    connect(m_prevMainPropShortcut,  &QShortcut::activated, scene, &Scene::prevMainPropShortcut);
    connect(m_nextMainPropShortcut,  &QShortcut::activated, scene, &Scene::nextMainPropShortcut);
    connect(m_prevSecndPropShortcut, &QShortcut::activated, scene, &Scene::prevSecndPropShortcut);
    connect(m_nextSecndPropShortcut, &QShortcut::activated, scene, &Scene::nextSecndPropShortcut);
    connect(m_changePrevElmShortcut, &QShortcut::activated, scene, &Scene::prevElm);
    connect(m_changeNextElmShortcut, &QShortcut::activated, scene, &Scene::nextElm);
    connect(scene, &Scene::openTruthTableRequested, this, [this] {
        m_ui->elementEditor->truthTable();
    });
    connect(scene, &Scene::icOpenRequested, this, [this](int elementId, const QString &blobName, const QString &filePath) {
        if (!blobName.isEmpty()) {
            if (m_bound) {
                emit openICRequested(blobName, elementId, m_bound->scene()->icRegistry()->blob(blobName));
            }
        } else if (!filePath.isEmpty()) {
            emit loadFileRequested(filePath);
        }
    });
    connect(scene, &Scene::icPreviewRequested, this, [this](IC *ic, const QPoint &screenPos) {
        m_previewPopup->showForIC(ic, screenPos);
    });
    connect(scene, &Scene::icPreviewMoved, this, [this](IC *ic, const QPoint &screenPos) {
        // Keep tracking the cursor while a show is pending, but re-arm the show
        // when the cursor returns from a port within the same hover.
        if (m_previewPopup->isShowActiveFor(ic)) {
            m_previewPopup->updatePendingPos(screenPos);
        } else {
            m_previewPopup->showForIC(ic, screenPos);
        }
    });
    connect(scene, &Scene::icPreviewHideRequested, this, [this] {
        m_previewPopup->scheduleHide();
    });
    connect(scene, &Scene::icPreviewCancelRequested, this, [this](IC *ic) {
        Q_UNUSED(ic)
        // Unconditional, matching the pre-signal direct-call behavior: double-clicking
        // any IC hides the one shared popup regardless of which IC it was pending/showing
        // for (harmless no-op if it wasn't visible). A pendingIC()-gated version would
        // leave a stale preview on screen if the user switched tabs while a popup from
        // another tab's IC was still pending/visible (tab switch never touches the popup).
        m_previewPopup->cancelHide();
        m_previewPopup->hide();
    });

    if (m_ui->actionPlay->isChecked()) {
        qCDebug(zero) << "Restarting simulation.";
        tab->simulation()->start();
        // Clear selection so the element editor doesn't show stale data from the
        // previously active tab.
        scene->clearSelection();
    }

    tab->view()->setFastMode(m_ui->actionFastMode->isChecked());
    // Synchronise zoom button state to the newly visible tab's zoom level.
    m_ui->actionZoomIn->setEnabled(tab->view()->canZoomIn());
    m_ui->actionZoomOut->setEnabled(tab->view()->canZoomOut());

    // Synchronise the mute button state to the newly visible tab's mute intent.
    const bool muted = tab->simulation()->isUserMuted();
    m_ui->actionMute->setChecked(muted);
    m_ui->actionMute->setText(muted ? tr("Unmute") : tr("Mute"));
}

void SceneUiBinder::unbind()
{
    // Tear down all connections that route scene signals into shared UI elements, and
    // stop the simulation so it doesn't keep running in the background consuming CPU.
    if (!m_bound) {
        return;
    }

    auto *scene = m_bound->scene();

    m_ui->elementEditor->setScene(nullptr);

    qCDebug(zero) << "Stopping simulation.";
    m_bound->simulation()->stop();

    qCDebug(zero) << "Disconnecting zoom and simulation signals from UI.";
    disconnect(m_bound->view(),       &GraphicsView::zoomChanged,     this, &SceneUiBinder::syncZoomActions);
    disconnect(m_bound->simulation(), &Simulation::simulationWarning, this, nullptr);

    qCDebug(zero) << "Disconnecting scene shortcuts from previous tab.";
    disconnect(m_prevMainPropShortcut,  nullptr, scene, nullptr);
    disconnect(m_nextMainPropShortcut,  nullptr, scene, nullptr);
    disconnect(m_prevSecndPropShortcut, nullptr, scene, nullptr);
    disconnect(m_nextSecndPropShortcut, nullptr, scene, nullptr);
    disconnect(m_changePrevElmShortcut, nullptr, scene, nullptr);
    disconnect(m_changeNextElmShortcut, nullptr, scene, nullptr);
    disconnect(scene, &Scene::openTruthTableRequested, this, nullptr);
    disconnect(scene, &Scene::icOpenRequested, this, nullptr);
    disconnect(scene, &Scene::icPreviewRequested, this, nullptr);
    disconnect(scene, &Scene::icPreviewMoved, this, nullptr);
    disconnect(scene, &Scene::icPreviewHideRequested, this, nullptr);
    disconnect(scene, &Scene::icPreviewCancelRequested, this, nullptr);
    disconnect(scene->undoStack(), &QUndoStack::indexChanged, this, nullptr);

    qCDebug(zero) << "Removing undo and redo actions from UI menu.";
    removeUndoRedoMenu();

    m_bound = nullptr;
}
