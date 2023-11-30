import os
from conftest import BINDIR


def run_command(cmd: str):
    x = os.system(cmd)
    return (x & 0xff00) >> 8


def test_basic_gossip():
    cmd = f'{BINDIR}/test_gossip_basic'
    assert(run_command(cmd) == 0)
    pass


def test_adv_gossip():
    cmd = f'{BINDIR}/test_gossip_adv'
    assert(run_command(cmd) == 0)
    pass
