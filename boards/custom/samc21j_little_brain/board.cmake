# Copyright (c) 2022 Kamil Serwus
# SPDX-License-Identifier: Apache-2.0

board_runner_args(openocd --cmd-pre-init "source [find interface/cmsis-dap.cfg]")
board_runner_args(openocd --cmd-pre-init "transport select swd")
board_runner_args(openocd --cmd-pre-init "source [find target/at91samdXX.cfg]")

#include(${ZEPHYR_BASE}/boards/common/openocd.boardname.board.cmake)

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)