.. _cli_usage:

Basic usage
===========

Typical commandline usage
-------------------------

The simplest way to use the program is ``starcry <script> [ <file> ]``.

+---------------------------------+---------------------------------------------------------------------+
| **Basic usage**                 |                                                                     |
+=================================+=====================================================================+
| ``starcry test.js``             | Render video using ``test.js`` to default location ``output.h264``. |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry test.js test.h264``   | Render video using ``test.js`` to location ``test.h264``.           |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry test.js --gui``       | Render video and launch UI that shows rendering progress.           |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry --spawn-gui``         | Launch UI manually this way.                                        |
+---------------------------------+---------------------------------------------------------------------+
| **Fine-tuning**                 |                                                                     |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry ... -n 8``            | Render using 8 *local* threads (instead of default of 4).           |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry ... -c 4``            | Split frames into 4 chunks (or *job*) (instead of 1 per frame).     |
+---------------------------------+---------------------------------------------------------------------+
| **Using remote workers**        |                                                                     |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry -w 10000``            | Launch a worker on port 10000.                                      |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry ... -r servers.txt``  | Render using workers specified in ``servers.txt``.                  |
+---------------------------------+---------------------------------------------------------------------+
| ``starcry ... --compress``      | Compress (useful for sending large frame sizes over the network)    |
+---------------------------------+---------------------------------------------------------------------+

**The GUI (---gui, ---spawn-gui)**

The GUI binds to a any available port and writes this information to ``$HOME/.starcry.conf``.
Via this configuration file successive starcry instances can re-use the same UI window.

**Setting parameters from within Javascript**

Inside the scripts a lot of variables can be set, which will be read by starcry before rendering starts.
This is the preferred way of specifying options as it keeps invoking starcry from the commandline cleaner.

As an example ``--stdin`` can be used to instruct starcry to read from standard input, as does the statement ``var stdin = true;`` in the javascript code.
The latter approach has the advantage that the user of your script (which may be you) can no longer forget to specify it in the commandline.

For more details view the :ref:`javascript_api`.

All CLI parameters
------------------

``starcry --help`` produces the following output.

.. literalinclude:: cli_help.txt

