import os
import subprocess
import importlib.util as iu

from benchmark_core import BenchMarker

# ======= Benchmark Imports ==========

SRC_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../src/python/code/gnoll/"))
m = os.path.join(SRC_DIR, "parser.py")
spec = iu.spec_from_file_location("dt", m)
dt = iu.module_from_spec(spec)
spec.loader.exec_module(dt)
gnoll_roll = dt.roll

troll_exec = os.path.join(
    os.path.expanduser('~'),
    "troll"
)
diceparser_exec = os.path.join(
    os.path.expanduser('~'),
    "diceparser"
)

def troll_roll(s):
    global troll_exec

    v = subprocess.run([troll_exec, "0", "test.t"], capture_output=True)
    if (v.returncode):
        raise ValueError

def dp_roll(s):
    global time1
    subprocess.run([diceparser_exec, s])



# ======= Benchmark Begins ==========
bm = BenchMarker(end_range=6)

bm.addFunction("GNOLL", gnoll_roll, color="b", marker="s")
bm.addFunction("TROLL", troll_roll, color="g", marker="o")
bm.addFunction("DiceParser", dp_roll, color="r", marker="^")

bm.benchmark("C/SML/C++ Library comparison")

this_folder = os.path.dirname(__file__)
output_file = os.path.join(this_folder, "../../doc/JOSS/C++.PNG")
bm.save(output_file)