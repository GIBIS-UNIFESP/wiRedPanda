#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create D Flip-Flop IC with Preset and Clear

Standard master-slave D flip-flop (textbook design):

  INPUT GATING (AND gates, active when CLK=0):
    AND_S = D     AND NOT_CLK   → S_master
    AND_R = NOT_D AND NOT_CLK   → R_master

  MASTER NOR SR LATCH (transparent when CLK=0):
    NOR_Q_m    = NOR(R_master, Q_bar_m, NOT_CLEAR)    → Q_master
    NOR_Qbar_m = NOR(S_master, Q_m,     NOT_PRESET)   → Q_bar_master

  CLOCK INVERTER:
    NOT_CLK = NOT(CLK)

  SLAVE GATING (AND gates, active when CLK=1):
    AND_S_slave = Q_master     AND CLK   → S_slave
    AND_R_slave = Q_bar_master AND CLK   → R_slave

  SLAVE NOR SR LATCH (transparent when CLK=1):
    NOR_Q_s    = NOR(R_slave_or, Q_bar_s)   → Q       (final output)
    NOR_Qbar_s = NOR(S_slave_or, Q_s)       → Q_bar   (final output)

  PRESET/CLEAR INJECTION (OR gates into slave):
    S_slave_or = S_slave OR NOT(Preset)   → forces Q_bar_s=0 → Q=1
    R_slave_or = R_slave OR NOT(Clear)    → forces Q_s=0    → Q=0

Timing: positive-edge triggered (captures on CLK 0→1).
  CLK=0: master transparent (follows D), slave latched.
  CLK=1: master latched, slave transparent (copies master).
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class DFlipFlopBuilder(ICBuilderBase):
    """Builder for D Flip-Flop IC using master-slave latches"""

    async def create(self) -> bool:
        await self.begin_build("D Flip-Flop")

        if not await self.create_new_circuit():
            return False

        HS = HORIZONTAL_GATE_SPACING
        VS = VERTICAL_STAGE_SPACING

        input_x = 50.0
        not_x = input_x + HS
        and_master_x = not_x + HS
        master_nor_x = and_master_x + HS
        and_slave_x = master_nor_x + HS
        or_slave_x = and_slave_x + HS
        slave_nor_x = or_slave_x + HS
        output_x = slave_nor_x + HS

        row1_y = 100.0                  # S path (D / set)
        row2_y = row1_y + VS            # R path (NOT D / reset)
        clk_y = row2_y + VS             # Clock
        preset_y = clk_y + VS           # Preset
        clear_y = preset_y + VS         # Clear

        # ========== Helper ==========
        async def mk(etype, x, y, label, input_size=None):
            r = await self.mcp.send_command("create_element", {
                "type": etype, "x": x, "y": y, "label": label
            })
            if not r or not r.success:
                print(f"❌ Failed to create {label}")
                return None
            eid = r.result.get('element_id')
            self.element_count += 1
            if input_size is not None:
                r2 = await self.mcp.send_command("set_element_properties", {
                    "element_id": eid, "input_size": input_size
                })
                if not r2 or not r2.success:
                    print(f"❌ Failed to resize {label} to {input_size} inputs")
                    return None
            return eid

        async def conn(src, sp, tgt, tp, desc=""):
            r = await self.mcp.send_command("connect_elements", {
                "source_id": src, "source_port": sp,
                "target_id": tgt, "target_port": tp
            })
            if not r or not r.success:
                print(f"❌ Failed to connect: {desc}")
                return False
            self.connection_count += 1
            return True

        # ========== Create elements ==========

        # Inputs
        d_id     = await mk("InputSwitch", input_x, row1_y, "D")
        clk_id   = await mk("InputSwitch", input_x, clk_y, "Clock")
        prst_id  = await mk("InputSwitch", input_x, preset_y, "Preset")
        clr_id   = await mk("InputSwitch", input_x, clear_y, "Clear")

        # Inverters
        not_d_id    = await mk("Not", not_x, row2_y, "not_d")
        not_clk_id  = await mk("Not", not_x, clk_y, "not_clk")
        not_prst_id = await mk("Not", not_x, preset_y, "not_preset")
        not_clr_id  = await mk("Not", not_x, clear_y, "not_clear")

        # Master input gating: AND_S = D · CLK, AND_R = NOT_D · CLK
        and_s_id = await mk("And", and_master_x, row1_y, "and_master_s")
        and_r_id = await mk("And", and_master_x, row2_y, "and_master_r")

        # Master NOR SR latch: 3-input (gate, feedback, async clear/preset)
        m_nor_q_id    = await mk("Nor", master_nor_x, row1_y, "master_nor_q", input_size=3)
        m_nor_qbar_id = await mk("Nor", master_nor_x, row2_y, "master_nor_qbar", input_size=3)

        # Slave gating: AND_S_slave = Q_master · CLK_bar, AND_R_slave = Q_bar_master · CLK_bar
        and_ss_id = await mk("And", and_slave_x, row1_y, "and_slave_s")
        and_sr_id = await mk("And", and_slave_x, row2_y, "and_slave_r")

        # OR gates for preset/clear injection into slave
        or_s_id = await mk("Or", or_slave_x, row1_y, "or_slave_s")
        or_r_id = await mk("Or", or_slave_x, row2_y, "or_slave_r")

        # Slave NOR SR latch: 2-input
        s_nor_q_id    = await mk("Nor", slave_nor_x, row1_y, "slave_nor_q")
        s_nor_qbar_id = await mk("Nor", slave_nor_x, row2_y, "slave_nor_qbar")

        # Outputs
        q_id    = await mk("Led", output_x, row1_y, "Q")
        qbar_id = await mk("Led", output_x, row2_y, "Q_bar")

        if None in (d_id, clk_id, prst_id, clr_id, not_d_id, not_clk_id,
                    not_prst_id, not_clr_id, and_s_id, and_r_id,
                    m_nor_q_id, m_nor_qbar_id, and_ss_id, and_sr_id,
                    or_s_id, or_r_id, s_nor_q_id, s_nor_qbar_id, q_id, qbar_id):
            return False

        # ========== Wiring ==========

        # Inverters
        if not await conn(d_id, 0, not_d_id, 0, "D→NOT_D"): return False
        if not await conn(clk_id, 0, not_clk_id, 0, "CLK→NOT_CLK"): return False
        if not await conn(prst_id, 0, not_prst_id, 0, "Preset→NOT"): return False
        if not await conn(clr_id, 0, not_clr_id, 0, "Clear→NOT"): return False

        # Master input gating: AND_S = D · NOT_CLK (transparent when CLK=0)
        if not await conn(d_id, 0, and_s_id, 0, "D→AND_S"): return False
        if not await conn(not_clk_id, 0, and_s_id, 1, "NOT_CLK→AND_S"): return False

        # Master input gating: AND_R = NOT_D · NOT_CLK
        if not await conn(not_d_id, 0, and_r_id, 0, "NOT_D→AND_R"): return False
        if not await conn(not_clk_id, 0, and_r_id, 1, "NOT_CLK→AND_R"): return False

        # Master NOR Q = NOR(R_master, Q_bar_master, NOT_CLEAR)
        if not await conn(and_r_id, 0, m_nor_q_id, 0, "AND_R→NOR_Q_m"): return False
        if not await conn(m_nor_qbar_id, 0, m_nor_q_id, 1, "Q'_m→NOR_Q_m"): return False
        if not await conn(not_clr_id, 0, m_nor_q_id, 2, "NOT_CLR→NOR_Q_m"): return False

        # Master NOR Q_bar = NOR(S_master, Q_master, NOT_PRESET)
        if not await conn(and_s_id, 0, m_nor_qbar_id, 0, "AND_S→NOR_Q'_m"): return False
        if not await conn(m_nor_q_id, 0, m_nor_qbar_id, 1, "Q_m→NOR_Q'_m"): return False
        if not await conn(not_prst_id, 0, m_nor_qbar_id, 2, "NOT_PRST→NOR_Q'_m"): return False

        # Slave gating: AND_S_slave = Q_master · CLK (transparent when CLK=1)
        if not await conn(m_nor_q_id, 0, and_ss_id, 0, "Q_m→AND_SS"): return False
        if not await conn(clk_id, 0, and_ss_id, 1, "CLK→AND_SS"): return False

        # Slave gating: AND_R_slave = Q_bar_master · CLK
        if not await conn(m_nor_qbar_id, 0, and_sr_id, 0, "Q'_m→AND_SR"): return False
        if not await conn(clk_id, 0, and_sr_id, 1, "CLK→AND_SR"): return False

        # OR gates for preset/clear injection
        # or_s = AND_S_slave OR NOT_PRESET → drives slave NOR_Qbar
        if not await conn(and_ss_id, 0, or_s_id, 0, "AND_SS→OR_S"): return False
        if not await conn(not_prst_id, 0, or_s_id, 1, "NOT_PRST→OR_S"): return False

        # or_r = AND_R_slave OR NOT_CLEAR → drives slave NOR_Q
        if not await conn(and_sr_id, 0, or_r_id, 0, "AND_SR→OR_R"): return False
        if not await conn(not_clr_id, 0, or_r_id, 1, "NOT_CLR→OR_R"): return False

        # Slave NOR Q = NOR(or_r, Q_bar_slave)
        if not await conn(or_r_id, 0, s_nor_q_id, 0, "OR_R→NOR_Q_s"): return False
        if not await conn(s_nor_qbar_id, 0, s_nor_q_id, 1, "Q'_s→NOR_Q_s"): return False

        # Slave NOR Q_bar = NOR(or_s, Q_slave)
        if not await conn(or_s_id, 0, s_nor_qbar_id, 0, "OR_S→NOR_Q'_s"): return False
        if not await conn(s_nor_q_id, 0, s_nor_qbar_id, 1, "Q_s→NOR_Q'_s"): return False

        # Outputs
        if not await conn(s_nor_q_id, 0, q_id, 0, "Q_s→Q_LED"): return False
        if not await conn(s_nor_qbar_id, 0, qbar_id, 0, "Q'_s→Q'_LED"): return False

        # Save
        output_file = str(IC_COMPONENTS_DIR / "level1_d_flip_flop.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created DFlipFlop IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    builder = DFlipFlopBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "D Flip-Flop IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
