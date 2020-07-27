"""
Python interface to collectdb.h
This file shoulb be buid with Cython

To build without hassle:
        pyflags=`pkg-config --cflags --libs python3`
    will collect required flags

        gcc -O2 -I../inc -fPIC -lsqlite3 ../src/collectdb.c -c
    will compile .o of the used functions

        cython -3 -I../inc pycollectdb.pyx
    will produce .c representation

        gcc -O2 ${pyflags} -I../inc -fPIC pycollectdb.c -c
    will compile .o of cythonized extension

        gcc -O2 -shared -fPIC -lsqlite3 ${pyflags} \
        pycollectdb.o collectd.o -o pycollectdb.so
    will build Python C extension

"""

from libc.time cimport time,time_t

cdef extern from "collectdb.h":

    cpdef enum cerror:
        CE_OK
        CE_OE
        CE_EE
        CE_SE
        __CE_LAST

    cdef cerror print_db_table(char *db_name, time_t _from, time_t _to)

class CollectDBError(Exception):
    def __init__(self, cerror error, *args):
        err=cerror(error)
        super().__init__(*args, err)

class CollectTst:
    # def __init__(self):

#def _print_db_table(const char *db_name, cdef time_t _from, cdef time_t _to):
    def _print_db_table(self, const char *db_name, time_t _from, time_t _to):
        cdef cerror error = cerror.CE_OK
        error = print_db_table(db_name, _from, _to)
        if error != cerror.CE_OK:
            raise CollectDBError(error)
