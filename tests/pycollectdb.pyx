
#from clib.time cimport time_t
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
    def _print_db_table(const char *db_name, time_t _from, time_t _to):
        cdef cerror error = cerror.CE_OK
        if error != cerror.CE_OK:
            raise CollectDBError(error)
