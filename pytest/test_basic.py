import os
from conftest import BINDIR


def run_command(cmd: str):
    x = os.system(cmd)
    return (x & 0xff00) >> 8


def test_basic_dafka():
    cmd = f'{BINDIR}/test_basic'
    assert(run_command(cmd) == 0)
    pass
