import sys
import os
import subprocess

PY_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../src/python'))
sys.path.append(PY_DIR)
from yacc_wrapper import roll


def test_roll(s, mock_random=None):
    target_file = os.path.join(PY_DIR, "../grammar/dice.yacc")
    if mock_random is not None:

        replacements = [
            "return rand()%(big+1-small)+small;",
            "return rand()%(length_of_symbolic_array);"
        ]
        if isinstance(mock_random, tuple):
            new_code = ""
            for i, x in enumerate(mock_random):
                new_code += f"""
                    if(random_mock_count == {i}){{
                        random_mock_count++;
                        return {x};
                    }}
                """
        else:
            new_code = f"return {mock_random};"

        with open(target_file,'r',encoding='utf-8') as file:
            data = file.readlines()

        for x in range(len(data)):
            for r in replacements:
                if r in data[x]:
                    data[x] = new_code

        target_file = os.path.join(PY_DIR, "../grammar/test_dice.yacc")

        with open(target_file,'w',encoding='utf-8') as file:
            file.writelines(data)
    
    cmd = "make __all"
    parser = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    return roll(s)