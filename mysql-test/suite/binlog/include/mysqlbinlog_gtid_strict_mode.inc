#
# This file runs test cases for using --gtid-strict-mode with mariadb-binlog to
# ensure warnings are properly displayed
#
# param $is_strict_mode   boolean (0 for false, 1 for true) to enable or
#                         disable strict mode for GTID processing
#

--let MYSQLD_DATADIR=`select @@datadir`
--let OUT_FILE=$MYSQLTEST_VARDIR/tmp/binlog.out

if ($is_strict_mode == 0)
{
  --let BINLOG_STRICT_MODE_PARAM=--skip-gtid-strict-mode
}
if ($is_strict_mode == 1)
{
  --let BINLOG_STRICT_MODE_PARAM=--gtid-strict-mode
}

--let $log_error_ = $MYSQLTEST_VARDIR/tmp/out.err
--let SEARCH_FILE=$log_error_

--echo #
--echo #   Test Case 1:
--echo #   Sequential sequence numbers results in no warnings
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
INSERT INTO t1 values (2);
INSERT INTO t1 values (3);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 2:
--echo #   A skipped sequence number results in no warnings if all numbers are
--echo # monotonic (i.e. gaps in sequence number are allowed provided they
--echo # never decrease)
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 5;
INSERT INTO t1 values (3);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 3:
--echo #   A sequence number lower than the last processed value results in a
--echo # warning
CREATE TABLE t1 (a int);
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
SET @@session.gtid_seq_no= 2;
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 1;
INSERT INTO t1 values (1);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 4:
--echo #   Skipping a GTID and later receiving it results in a warning
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
SET @@session.gtid_seq_no= 3;
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 2;
INSERT INTO t1 values (3);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (4);
INSERT INTO t1 values (5);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 5:
--echo #   Repeat sequence numbers produce a warning
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
SET @@session.gtid_seq_no= 2;
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 2;
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 3;
INSERT INTO t1 values (3);
INSERT INTO t1 values (4);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 6:
--echo #   Warnings from different domains are all displayed
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
SET @@session.gtid_seq_no= 3;
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 2;
INSERT INTO t1 values (3);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (4);
SET @@session.gtid_domain_id= 1;
SET @@session.server_id= 2;
CREATE TABLE t2 (a int);
SET @@session.gtid_seq_no= 3;
INSERT INTO t2 values (2);
SET @@session.gtid_seq_no= 2;
INSERT INTO t2 values (3);
SET @@session.gtid_seq_no= 4;
INSERT INTO t2 values (4);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
DROP TABLE t2;
RESET MASTER;

--echo #
--echo #   Test Case 7:
--echo #   A decreasing seq_no before a start-position is ignored
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
SET @@session.gtid_seq_no= 1;
CREATE TABLE t1 (a int);
SET @@session.gtid_seq_no= 3;
INSERT INTO t1 values (1);
SET @@session.gtid_seq_no= 2;
INSERT INTO t1 values (2);
--let $start_binlog_pos= query_get_value(SHOW MASTER STATUS,Position, 1)
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (4);
INSERT INTO t1 values (3);
INSERT INTO t1 values (5);
FLUSH LOGS;

--echo # GTID-based start-position
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-4 $BINLOG_STRICT_MODE_PARAM > log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-4 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc

--echo # Position-based start-position
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=start_binlog_pos $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=$start_binlog_pos $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc

--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 8:
--echo #   A decreasing seq_no inside of a --start/--stop position window is
--echo # displayed
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
--let $start_binlog_pos= query_get_value(SHOW MASTER STATUS,Position, 1)
INSERT INTO t1 values (1);
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 5;
INSERT INTO t1 values (4);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (3);
SET @@session.gtid_seq_no= 6;
INSERT INTO t1 values (5);
--let $stop_binlog_pos= query_get_value(SHOW MASTER STATUS,Position, 1)
FLUSH LOGS;

--echo # GTID-based window
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-1 --stop-position=0-1-6 $BINLOG_STRICT_MODE_PARAM > log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-1 --stop-position=0-1-6 $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc

--echo # Position-based window
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=start_binlog_pos --stop-position=stop_binlog_pos $BINLOG_STRICT_MODE_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=$start_binlog_pos --stop-position=$stop_binlog_pos $BINLOG_STRICT_MODE_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
--remove_file $log_error_
DROP TABLE t1;
RESET MASTER;

--echo #
--echo #   Test Case 9:
--echo #   Error if --stop-position is not strictly greater than
--echo # --start-position
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-2 --stop-position=0-1-1 $BINLOG_STRICT_MODE_PARAM > OUT_FILE
--error $is_strict_mode
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-2 --stop-position=0-1-1 $BINLOG_STRICT_MODE_PARAM > $OUT_FILE

--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-1 --stop-position=0-1-1 $BINLOG_STRICT_MODE_PARAM > OUT_FILE
--error $is_strict_mode
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-1 --stop-position=0-1-1 $BINLOG_STRICT_MODE_PARAM > $OUT_FILE

--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-2,1-2-1 --stop-position=0-1-1,1-2-2 $BINLOG_STRICT_MODE_PARAM > OUT_FILE
--error $is_strict_mode
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-2,1-2-1 --stop-position=0-1-1,1-2-2 $BINLOG_STRICT_MODE_PARAM > $OUT_FILE
RESET MASTER;

--remove_file $OUT_FILE
