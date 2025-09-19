// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "handlerfactory.h"
#include "../handlers/connectionhandler.h"
#include "../handlers/elementhandler.h"
#include "../handlers/filehandler.h"
#include "../handlers/serverinfohandler.h"
#include "../handlers/simulationhandler.h"

#include <QDebug>

HandlerFactory HandlerFactory::s_instance;

HandlerFactory& HandlerFactory::instance()
{
    return s_instance;
}

bool HandlerFactory::registerHandler(const QString &commandName,
                                     const QString &category,
                                     const HandlerCreator &creator)
{
    if (m_handlers.contains(commandName)) {
        qDebug() << "Command already registered:" << commandName;
        return false;
    }

    m_handlers[commandName] = HandlerInfo{category, creator};
    return true;
}

std::unique_ptr<BaseHandler> HandlerFactory::createHandler(const QString &commandName,
                                                           MainWindow *mainWindow,
                                                           MCPValidator *validator) const
{
    auto it = m_handlers.find(commandName);
    if (it == m_handlers.end()) {
        return nullptr;
    }

    return it->creator(mainWindow, validator);
}

bool HandlerFactory::isCommandRegistered(const QString &commandName) const
{
    return m_handlers.contains(commandName);
}

QStringList HandlerFactory::getRegisteredCommands() const
{
    return m_handlers.keys();
}

QStringList HandlerFactory::getCommandsByCategory(const QString &category) const
{
    QStringList commands;
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
        if (it->category == category) {
            commands.append(it.key());
        }
    }
    return commands;
}

QStringList HandlerFactory::getCategories() const
{
    QStringList categories;
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
        if (!categories.contains(it->category)) {
            categories.append(it->category);
        }
    }
    return categories;
}

QString HandlerFactory::getCommandCategory(const QString &commandName) const
{
    auto it = m_handlers.find(commandName);
    if (it != m_handlers.end()) {
        return it->category;
    }
    return QString();
}

bool HandlerFactory::unregisterHandler(const QString &commandName)
{
    return m_handlers.remove(commandName) > 0;
}

void HandlerFactory::clear()
{
    m_handlers.clear();
}

int HandlerFactory::getRegisteredCommandCount() const
{
    return m_handlers.size();
}

void HandlerFactory::registerBuiltInHandlers()
{
    // Server info commands
    registerHandler("get_server_info", "server",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ServerInfoHandler>(mainWindow, validator);
                    });

    // File operation commands
    registerHandler("load_circuit", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });
    registerHandler("save_circuit", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });
    registerHandler("new_circuit", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });
    registerHandler("close_circuit", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });
    registerHandler("get_tab_count", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });
    registerHandler("export_image", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });
    registerHandler("export_arduino", "file",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<FileHandler>(mainWindow, validator);
                    });

    // Element operation commands
    registerHandler("create_element", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });
    registerHandler("delete_element", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });
    registerHandler("list_elements", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });
    registerHandler("move_element", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });
    registerHandler("set_element_properties", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });
    registerHandler("set_input_value", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });
    registerHandler("get_output_value", "element",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ElementHandler>(mainWindow, validator);
                    });

    // Connection operation commands
    registerHandler("connect_elements", "connection",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ConnectionHandler>(mainWindow, validator);
                    });
    registerHandler("disconnect_elements", "connection",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ConnectionHandler>(mainWindow, validator);
                    });
    registerHandler("list_connections", "connection",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<ConnectionHandler>(mainWindow, validator);
                    });

    // Simulation operation commands
    registerHandler("simulation_control", "simulation",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<SimulationHandler>(mainWindow, validator);
                    });
    registerHandler("create_waveform", "simulation",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<SimulationHandler>(mainWindow, validator);
                    });
    registerHandler("export_waveform", "simulation",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<SimulationHandler>(mainWindow, validator);
                    });
    registerHandler("create_ic", "simulation",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<SimulationHandler>(mainWindow, validator);
                    });
    registerHandler("instantiate_ic", "simulation",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<SimulationHandler>(mainWindow, validator);
                    });
    registerHandler("list_ics", "simulation",
                    [](MainWindow *mainWindow, MCPValidator *validator) {
                        return std::make_unique<SimulationHandler>(mainWindow, validator);
                    });
}
