# !/bin/sh

../../tools/syn_to_c builtin.common >builtin.common.inc
../../tools/syn_to_c builtin.frag >builtin.frag.inc
../../tools/syn_to_c builtin.init.frag >builtin.init.frag.inc
../../tools/syn_to_c builtin.vert >builtin.vert.inc
../../tools/syn_to_c builtin.init.vert >builtin.init.vert.inc
