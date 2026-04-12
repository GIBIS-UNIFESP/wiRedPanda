// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Core/HandlerFactory.h"

#include "MCP/Server/Handlers/ConnectionHandler.h"
#include "MCP/Server/Handlers/ElementHandler.h"
#include "MCP/Server/Handlers/FileHandler.h"
#include "MCP/Server/Handlers/ServerInfoHandler.h"
#include "MCP/Server/Handlers/SimulationHandler.h"

HandlerFactory HandlerFactory::s_instance;

HandlerFactory &HandlerFactory::instance()
{
    return s_instance;
}

bool HandlerFactory::registerHandler(const QString &commandName,
                                     const QString &category,
                                     const HandlerCreator &creator)
{
    if (m_handlers.contains(commandName)) {
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
    return m_handlers.remove(commandName) != 0;
}

void HandlerFactory::clear()
{
    m_handlers.clear();
}

int HandlerFactory::getRegisteredCommandCount() const
{
    return static_cast<int>(m_handlers.size());
}

void HandlerFactory::registerBuiltInHandlers()
{
    registerHandlerGroup<ServerInfoHandler>("server", {
        "get_server_info"
    });

    registerHandlerGroup<FileHandler>("file", {
        "load_circuit", "save_circuit", "new_circuit", "close_circuit",
        "get_tab_count", "export_image"
    });

    registerHandlerGroup<ElementHandler>("element", {
        "create_element", "delete_element", "list_elements", "move_element",
        "set_element_properties", "set_input_value", "get_output_value"
    });

    registerHandlerGroup<ConnectionHandler>("connection", {
        "connect_elements", "disconnect_elements", "list_connections"
    });

    registerHandlerGroup<SimulationHandler>("simulation", {
        "simulation_control", "create_waveform", "export_waveform",
        "create_ic", "instantiate_ic", "list_ics",
        "embed_ic", "extract_ic"
    });
}

