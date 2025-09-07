#!/usr/bin/env python3
"""
Minimal MCP Test - Debug the Windows threading issue
Just tests the welcome message + first command flow
"""

import asyncio
import sys
from typing import Dict, Any

from beartype import beartype
from pydantic import BaseModel, Field

# Fix encoding for Windows console output
if sys.platform == "win32" and hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding="utf-8")  # type: ignore[attr-defined]
if sys.platform == "win32" and hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding="utf-8")  # type: ignore[attr-defined]

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class MCPTestConfig(BaseModel):
    """Configuration model for MCP test execution."""

    enable_validation: bool = Field(
        default=False,
        description="Enable schema validation during testing"
    )
    verbose: bool = Field(
        default=True,
        description="Enable verbose output for debugging"
    )

    class Config:
        """Pydantic configuration."""
        frozen = True
        extra = "forbid"


@beartype
async def main() -> bool:
    print("🔬 Minimal MCP Test - Debugging Windows threading issue")
    print("=" * 60)

    # Create validated configuration
    config = MCPTestConfig(enable_validation=False, verbose=True)

    # Create infrastructure with validated config
    mcp = MCPInfrastructure(enable_validation=config.enable_validation, verbose=config.verbose)

    try:
        print("\n🚀 Step 1: Starting MCP process...")
        if not await mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        print("✅ MCP process started successfully")

        print("\n📤 Step 2: Sending get_server_info command...")
        response = await mcp.send_command("get_server_info", {})
        if response and response.success:
            result = response.result if hasattr(response, 'result') else {}
            server_name = result.get('server_name', 'Unknown') if result else 'Unknown'
            print(f"✅ Server info received: {server_name}")
        else:
            print(f"❌ get_server_info failed: {response.error if response else 'No response'}")
            return False

        print("\n📤 Step 3: Sending get_tab_count command...")
        response = await mcp.send_command("get_tab_count", {})

        if response and response.success:
            print(f"✅ get_tab_count succeeded: {response.result}")
        else:
            print(f"❌ get_tab_count failed: {response.error if response else 'No response'}")
            return False

        print("\n📤 Step 4: Sending new_circuit command...")
        response = await mcp.send_command("new_circuit", {})

        if response and response.success:
            print(f"✅ new_circuit succeeded: {response.result}")
        else:
            print(f"❌ new_circuit failed: {response.error if response else 'No response'}")
            return False

        print("\n📤 Step 5: Sending get_tab_count again...")
        response = await mcp.send_command("get_tab_count", {})

        if response and response.success:
            print(f"✅ second get_tab_count succeeded: {response.result}")
        else:
            print(f"❌ second get_tab_count failed: {response.error if response else 'No response'}")
            return False

        print("\n✅ All commands succeeded - Windows threading is working!")
        return True

    except Exception as e:
        print(f"❌ Exception during test: {e}")
        import traceback
        traceback.print_exc()
        return False

    finally:
        print("\n🛑 Cleaning up...")
        await mcp.stop_mcp()
        print("✅ Cleanup complete")

if __name__ == "__main__":
    success = asyncio.run(main())
    print(f"\n{'✅ SUCCESS' if success else '❌ FAILED'}")
    sys.exit(0 if success else 1)
