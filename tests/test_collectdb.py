# endcoding=utf8

"""
Simple test for netcollect.c module
"""

import pycollectdb as cdb
import unittest as ut

class SimpleTestCollectdb(ut.TestCase):
    def test_print_table(self):
        c = cdb.CollectTst()
        c._print_db_table(b'../testdb/netstat.db', 0, 0)
