#
# This file runs warning cases for providing GTIDs to --start-position and
# --stop-position arguments in mariadb-binlog
#
# param $is_strict_mode   boolean (0 for false, 1 for true) to enable or
#                         disable strict mode for GTID processing
#

--let MYSQLD_DATADIR=`select @@datadir`
--let OUT_FILE=$MYSQLTEST_VARDIR/tmp/binlog.out

if ($is_strict_mode == 0)
{
    --let BINLOG_WARN_PARAM=--skip-gtid-strict-mode
}
if ($is_strict_mode == 1)
{
    --let BINLOG_WARN_PARAM=--gtid-strict-mode
}

--let $log_error_ = $MYSQLTEST_VARDIR/tmp/out.err
--let SEARCH_FILE=$log_error_

--echo #
--echo #   Warning Case 1:
--echo #   A gap in sequence number at end of window results in a warning
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
INSERT INTO t1 values (2);
SET @@session.gtid_seq_no= 5;
INSERT INTO t1 values (3);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0 --stop-position=0-1-4 $BINLOG_WARN_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0 --stop-position=0-1-4 $BINLOG_WARN_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;

--echo #
--echo #   Warning Case 2:
--echo #   A gap in sequence number at beginning of window results in a warning
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
SET @@session.gtid_seq_no= 3;
INSERT INTO t1 values (1);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-1 $BINLOG_WARN_PARAM > log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-1 $BINLOG_WARN_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;

--echo #
--echo #   Warning Case 3:
--echo #   A gap in sequence number within a window results in a warning
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (2);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0 --stop-position=0-1-4 $BINLOG_WARN_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0 --stop-position=0-1-4 $BINLOG_WARN_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;

--echo #
--echo #   Warning Case 4:
--echo #   When invoked with just --start-position, a gap in sequence number
--echo # after the provided GTID results in a warning
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (2);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0 $BINLOG_WARN_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0 $BINLOG_WARN_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;

--echo #
--echo #   Warning Case 5:
--echo #   When invoked with just --stop-position, a gap in sequence number
--echo # before the provided GTID results in a warning
RESET MASTER;
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (2);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --stop-position=0-1-4 $BINLOG_WARN_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --stop-position=0-1-4 $BINLOG_WARN_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;

--echo #
--echo #   Warning Case 6:
--echo #   One domain has a sequence number gap but another does not results in
--echo # a warning
RESET MASTER;
SET @@session.gtid_domain_id= 1;
SET @@session.server_id= 2;
CREATE TABLE t2 (a int);
INSERT INTO t2 values (1);
INSERT INTO t2 values (2);
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1);
SET @@session.gtid_seq_no= 4;
INSERT INTO t1 values (2);
FLUSH LOGS;
--echo # MYSQL_BINLOG MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0,1-2-0 --stop-position=0-1-4,1-2-3 $BINLOG_WARN_PARAM 2> log_error_ > OUT_FILE
--exec $MYSQL_BINLOG $MYSQLD_DATADIR/master-bin.000001 --start-position=0-1-0,1-2-0 --stop-position=0-1-4,1-2-3 $BINLOG_WARN_PARAM 2> $log_error_ > $OUT_FILE
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;
DROP TABLE t2;