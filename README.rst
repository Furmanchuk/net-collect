net-collect [under development]
-----------------------------------------------------------------------

CLI program for collecting statistic of internet traffic. It is possible to collect statistics for a certain period of time and displaying it in to the terminal. It is also possible to call a internal command to notify you that the limit has been exceeded.

Building instruction
-----------------------------------------------------------------------
Building and compiling use the *Makefile*


.. code-block:: bash

    #clone the repository
    git clone https://github.com/Furmanchuk/net-collect.git
    #build 
    make
    # or
    make all
    #to cleaning up
    make clean

Using example
-----------------------------------------------------------------------

- To run program in daemon mode:

.. code-block:: bash

    # run with default options
    ./net-    collect daemon --db=dbpath.db --interface=wlp3s0
    # run from notification of exceeding the limit
    ./net-collect daemon --db=dbpath.db --interface=wlp3s0 --cmd="zenity --warning --text=\"Limit reached\""
    
- To run program in statistic mode:  

.. code-block:: bash

    # print all data about traffic
    ./net-collect stat --db=dbpath.db
    # or print for the specified time
    ./net-collect stat --db=dbpath.db --from="2020 07 14 18:30:00" --to="2020 07 14 20:40:00"

- To run program in create mode: 

.. code-block:: bash

    # create database file
    ./net-collect create --db=somedbfile.db

The test database is located in a folder: *testdb*

LICENCE
-----------------------------------------------------------------------
The MIT License (MIT)  Copyright © 2020 Vadym Furmanchuk

