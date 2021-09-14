#
# This file runs test cases for providing GTIDs to --start-position and
# --stop-position arguments in mariadb-binlog
#
# param $is_remote   boolean (0 for false, 1 for true) to perform a local file
#                    or remote host analysis
#

--let MYSQLD_DATADIR=`select @@datadir`
--let OUT_FILE=$MYSQLTEST_VARDIR/tmp/binlog.out
--let BINLOG_FILE=master-bin.000001
--let data_inconsistent_err= "table data is inconsistent after replaying binlog using GTID start/stop positions";

if ($is_remote == 0)
{
    --let BINLOG_FILE_PARAM=$MYSQLD_DATADIR/$BINLOG_FILE.orig
}
if ($is_remote == 1)
{
    --let BINLOG_FILE_PARAM=--read-from-remote-server $BINLOG_FILE
}

## Initialize test data
#
SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
CREATE TABLE t1 (a int);
INSERT INTO t1 values (1), (2);
--let test2_t1_good_checksum= `CHECKSUM TABLE t1`

SET @@session.gtid_domain_id= 1;
SET @@session.server_id= 2;
CREATE TABLE t2 (a int);
INSERT INTO t2 values (1);

SET @@session.gtid_domain_id= 0;
SET @@session.server_id= 1;
INSERT INTO t1 values (3), (4);
--let t1_final_checksum= `CHECKSUM TABLE t1`

SET @@session.gtid_domain_id= 1;
SET @@session.server_id= 2;
INSERT INTO t2 values (2);
--let test4_t2_good_checksum= `CHECKSUM TABLE t2`

SET @@session.server_id= 3;
INSERT INTO t2 values (3);
--let test3_t2_good_checksum= `CHECKSUM TABLE t2`

SET @@session.server_id= 2;
INSERT INTO t2 values (4);
--let t2_final_checksum= `CHECKSUM TABLE t2`

FLUSH LOGS;
if ($is_remote == 0)
{
    --copy_file $MYSQLD_DATADIR/master-bin.000001 $BINLOG_FILE_PARAM
}

DROP TABLE t1;
DROP TABLE t2;


--echo #
--echo #   Test Case 1:
--echo #   The end of the binlog file resets the server and domain id of the
--echo # session

# As gtid_domain_id and server_id should not change after reading binlog in GTID
# mode, change variables to otherwise-unused values to ensure they remain
--let $reset_gtid_domain_id = `select @@session.gtid_domain_id`
--let $reset_server_id = `select @@session.server_id`
SET @@session.gtid_domain_id= 10;
SET @@session.server_id= 20;

# Replay the binlog
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=0-1-2 | $MYSQL
--let $test_gtid_domain_id = `select @@session.gtid_domain_id`

# Ensure variables haven't changed
--let $assert_text = session gtid_domain_id should not change when reading binlog in GTID mode
--let $assert_cond = @@session.gtid_domain_id = 10
--source include/assert.inc
--let $assert_text = session server_id should not change when reading binlog in GTID mode
--let $assert_cond = @@session.server_id = 20
--source include/assert.inc

# Reset back to previous state
--eval SET @@session.gtid_domain_id= $reset_gtid_domain_id
--eval SET @@session.server_id= $reset_server_id
DROP TABLE t1;


--echo #
--echo #   Test Case 2:
--echo #   Single GTID range specified
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=0-1-2 | $MYSQL

if ($test2_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;


--echo #
--echo #   Test Case 3:
--echo #   Single GTID range with different server_ids
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=1-2-0 --stop-position=1-3-4 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=1-2-0 --stop-position=1-3-4 | $MYSQL

if ($test3_t2_good_checksum != `CHECKSUM TABLE t2`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;
DROP TABLE t2;


--echo #
--echo #   Test Case 4:
--echo #   Multiple GTID ranges specified
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0,1-2-0 --stop-position=0-1-3,1-2-3 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0,1-2-0 --stop-position=0-1-3,1-2-3 | $MYSQL

# Reuse checksum spot from test 4
if ($t1_final_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if ($test4_t2_good_checksum != `CHECKSUM TABLE t2`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;
DROP TABLE t2;


--echo #
--echo #   Test Case 5:
--echo #   Multiple GTID ranges specified where the domain ids are listed in
--echo # different orders between start/stop position
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --stop-position=0-1-3,1-2-3 --start-position=1-2-0,0-1-0 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --stop-position=0-1-3,1-2-3 --start-position=1-2-0,0-1-0 | $MYSQL

# Reuse checksum spot from test 4
if ($t1_final_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if ($test4_t2_good_checksum != `CHECKSUM TABLE t2`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;
DROP TABLE t2;


--echo #
--echo #   Test Case 6:
--echo #   Only start position specified
CREATE TABLE t1 (a int);
INSERT INTO t1 values (3), (4);
--let test6_t1_good_checksum= `CHECKSUM TABLE t1`
DROP TABLE t1;
CREATE TABLE t1 (a int);
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-2 | $MYSQL
if ($test6_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if ($t2_final_checksum != `CHECKSUM TABLE t2`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;
DROP TABLE t2;


--echo #
--echo #   Test Case 7:
--echo #   Only stop position specified
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --stop-position=0-1-2 | $MYSQL

# Reuse checksum spot from test 2
if ($test2_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;


--echo #
--echo #   Test Case 8:
--echo #   Seq_no=0 in --start-position includes all events for a domain
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0,1-2-0 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0,1-2-0 | $MYSQL
if ($t1_final_checksum != `CHECKSUM TABLE t1`)
{
    die "t1 data should be complete as binlog replay should include domain 0 entirely in results";
}
if ($t2_final_checksum != `CHECKSUM TABLE t2`)
{
    die "t2 data should be complete as binlog replay should include domain 1 entirely in results";
}
DROP TABLE t1;
DROP TABLE t2;

--echo #
--echo #   Test Case 9:
--echo #   Seq_no=0 in --stop-position excludes all events for a domain
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --stop-position=0-1-0,1-2-0 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --stop-position=0-1-0,1-2-0 | $MYSQL
if (0 < `SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'test' AND table_name = 't1'`)
{
    die "t1 should not exist as binlog replay should exclude domain 0 from results";
}
if (0 < `SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'test' AND table_name = 't2'`)
{
    die "t2 should not exist as binlog replay should exclude domain 1 from results";
}

--echo #
--echo #   Test Case 10:
--echo #   Output stops for all domain ids when all --stop-position GTID values
--echo # have been hit.
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --stop-position=0-1-2 | $MYSQL
if ($test2_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if (0 < `SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'test' AND table_name = 't2'`)
{
    die "t2 should not exist as binlog replay should exclude domain 1 from results";
}
DROP TABLE t1;

--echo #
--echo #   Test Case 11:
--echo #   All GTID events from other domains are printed until the
--echo # --stop-position values are hit
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --stop-position=1-3-4 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --stop-position=1-3-4 | $MYSQL
if ($t1_final_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if ($test3_t2_good_checksum != `CHECKSUM TABLE t2`)
{
    die $data_inconsistent_err;
}
DROP TABLE t1;
DROP TABLE t2;

--echo #   Test Case 12:
--echo #   Scalar and GTID values can be used together for stop or start
--echo # position
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=4 --stop-position=0-1-1 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=4 --stop-position=0-1-1 | $MYSQL
DROP TABLE t1;
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=512 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=512 | $MYSQL
DROP TABLE t1;

--echo #   Test Case 13:
--echo #   Start position is delayed within the binlog
CREATE TABLE t1 (a int);
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-1 --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-1 --stop-position=0-1-2 | $MYSQL
if ($test2_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if (0 < `SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'test' AND table_name = 't2'`)
{
    die "t2 should not exist as binlog replay should exclude domain 1 from results";
}
--let $log_error_ = $MYSQLTEST_VARDIR/log/mysqld.1.err
--let SEARCH_FILE=$log_error_
--let SEARCH_PATTERN=WARNING
--source include/search_pattern_in_file.inc
DROP TABLE t1;

--echo #   Test Case 14:
--echo #   Start position is specified twice on the command line
CREATE TABLE t1 (a int);
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0 --start-position=0-1-1 --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0 --start-position=0-1-1 --stop-position=0-1-2 | $MYSQL
if ($test2_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if (0 < `SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'test' AND table_name = 't2'`)
{
    die "t2 should not exist as binlog replay should exclude domain 1 from results";
}
DROP TABLE t1;

--echo #   Test Case 15:
--echo #   Stop position is specified twice on the command line
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=0-1-1 --stop-position=0-1-2 | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM --start-position=0-1-0 --stop-position=0-1-1 --stop-position=0-1-2 | $MYSQL
if ($test2_t1_good_checksum != `CHECKSUM TABLE t1`)
{
    die $data_inconsistent_err;
}
if (0 < `SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'test' AND table_name = 't2'`)
{
    die "t2 should not exist as binlog replay should exclude domain 1 from results";
}
DROP TABLE t1;