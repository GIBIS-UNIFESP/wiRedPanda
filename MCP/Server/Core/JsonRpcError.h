// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * \file JsonRpcError.h
 * \brief Numeric error codes returned in MCP responses, per JSON-RPC 2.0.
 *
 * The first block reproduces the codes the JSON-RPC 2.0 spec reserves for
 * pre-defined errors (https://www.jsonrpc.org/specification#error_object).
 * The second block defines wiRedPanda application errors inside the spec's
 * reserved -32000..-32099 implementation-defined range.
 */
namespace JsonRpcError {

// JSON-RPC 2.0 pre-defined error codes
constexpr int ParseError     = -32700;  ///< Invalid JSON received by the server.
constexpr int InvalidRequest = -32600;  ///< The JSON sent is not a valid Request object.
constexpr int MethodNotFound = -32601;  ///< The requested method does not exist or is unavailable.
constexpr int InvalidParams  = -32602;  ///< Invalid method parameters (missing required, wrong type, etc.).
constexpr int InternalError  = -32603;  ///< Internal JSON-RPC error (last-resort).

// Application-defined codes (spec reserves -32000..-32099 for the implementation)
constexpr int SceneNotAvailable = -32000;  ///< No active circuit scene to operate on.
constexpr int ElementNotFound   = -32001;  ///< Referenced element id does not exist in the scene.
constexpr int ConnectionFailed  = -32002;  ///< Connect/disconnect or port-mismatch failure.
constexpr int FileError         = -32003;  ///< File save/load/path error.
constexpr int SimulationError   = -32004;  ///< Simulation control / waveform failure.
constexpr int ValidationError   = -32005;  ///< Semantic validation failure (e.g. port index out of range, enum value not allowed).
constexpr int IcError           = -32006;  ///< IC create / instantiate / embed / extract failure.
constexpr int PortNotFound      = -32007;  ///< Port lookup by index or label did not match.
constexpr int OperationFailed   = -32008;  ///< Generic Qt API / handler operation failure (catch-all for tryCommand).

}  // namespace JsonRpcError
