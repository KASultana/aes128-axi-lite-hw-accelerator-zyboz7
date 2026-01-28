# Third-Party Software Notices

This repository may include third-party open-source components. Where applicable, the original license headers have been preserved in the corresponding source files.

## AES-128 VHDL Core (`hardware/rtl/aes128key.vhd`)
The file `hardware/rtl/aes128key.vhd` includes an MIT license header referencing:
- Copyright (c) 2018 Balazs Valer Fekete
- Implementation based on NIST FIPS-197 (AES Standard)

This project integrates the AES core into a ZYBO Z7 (Zynq-7000) AXI-Lite hardware accelerator system (Vivado 2019.2 + Vitis) and uses it for HW/SW benchmarking.

**Note:** The authors took help from online resources during development but do not recall the exact original source and the exact extent of modifications (if any) made relative to the upstream version. To avoid misattribution, the license header inside `aes128key.vhd` is preserved as-is. The system integration work (AXI-Lite wrapper, PS software, and documentation) is authored by this repository’s contributors.
