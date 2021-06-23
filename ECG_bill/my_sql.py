import time
import threading
import traceback

import MySQLdb
from my_logger import MyLogger

log = MyLogger(debug_level=1)


class MyConnector(object):

    def __init__(self):

        self.dbl = 1
        self._conn = None
        self._db_host   = '127.0.0.1'
        self._db_name = 'clinic_active'
        self._db_port = 3307
        self._db_user   = 'mikenp'
        self._lock = threading.Lock()

    def set_db_name(self, name):
        self._db_name = name

    def connect(self):

        log.dbg(2, "called")

        if self._conn is not None:
            if self._conn.is_connected():
                log.dbg(2, "Already connected")
                return

        log.dbg(0, lambda: "Connecting to MySQL database: %s host: %s user: %s" %
                           (self._db_name, self._db_host, self._db_user))

        # NOTE: I could NOT get connections in seperate threads to see up-to-date
        # data until I added the "autocommit=True" param.... even when I called
        # commint on the connection object after database updates
        try:
            self._conn = MySQLdb.connect(
                host=self._db_host,
                database=self._db_name,
                user=self._db_user,
                port = self._db_port,
                autocommit=True)

            # if self._conn.is_connected():
            #     log.dbg(0, lambda: 'Connected to MySQL database %s' % self._db_name)

        except Exception as err:
            log.err("Exception connecting to MySQL database %s: %s" %
                (self._db_name, repr(err)))

    def get_cursor(self):
        log.dbg(2, "Called")

        # if self._conn is None or not self._conn.is_connected():
        #     self.connect()

        return self._conn.cursor(buffered=False)

    def execute(self, cmd, data, catch=True, rowcount=False):

        log.dbg(2, lambda: "Called; CMD: %s" % repr(cmd))
        log.dbg(2, lambda: "Called; DATA: %s" % repr(data))

        cursor = None
        affected_rowcount = 0
        last_row_id = 0

        if catch:
            try:
                self._lock.acquire()
                cursor = self.get_cursor()
                result = cursor.execute(cmd, data)
                # print("SQL EXECUTE RESUKT: %s" % repr(result))
                # print(dir(cursor))
                # print("============== CURSOR.ROWCOUNT", cursor.rowcount)
                affected_rowcount = cursor.rowcount
                last_row_id = cursor.lastrowid
                self._conn.commit()
                # print("SQL COMMIT RESUKT: %s" % repr(result))
                if rowcount:
                    return affected_rowcount

                if last_row_id != cursor.lastrowid:
                    print("LASTROWID ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")

                return cursor.lastrowid

            except Exception as err:
                log.err("Exception: %s CMD: %s" % (repr(err), repr(cmd)))
                print("*" * 80)
                trace = traceback.format_exc(10)
                print(trace)
                print("*" * 80)

            finally:
                if cursor:
                    log.dbg(2, lambda: "Closing cursor")
                    cursor.close()
                self._lock.release()
        else:
            try:
                self._lock.acquire()
                cursor = self.get_cursor()
                result = cursor.execute(cmd, data)
                # print("SQL EXECUTE RESUKT: %s" % repr(result))
                # print(dir(cursor))
                print("============== CURSOR.ROWCOUNT", cursor.rowcount)
                affected_rowcount = cursor.rowcount
                last_row_id = cursor.lastrowid

                result = self._conn.commit()
                # print("SQL COMMIT RESUKT: %s" % repr(result))

                if rowcount:
                    return affected_rowcount

                if last_row_id != cursor.lastrowid:
                    print("LASTROWID ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                return cursor.lastrowid

            finally:
                if cursor:
                    log.dbg(2, lambda: "Closing cursor")
                    cursor.close()
                self._lock.release()

    def select(self, cmd, catch=True):
        """
        Catch exceptions when running as background engine,
        raise exceptions when serving web pages
        """
        log.dbg(2, lambda: "called; SQL: %s" % repr(cmd))
        cursor = None

        start_time = time.time()
        if catch:
            try:
                self._lock.acquire()
                cursor = self.get_cursor()
                cursor.execute(cmd)
                result = cursor.fetchall()

                elapsed_time = time.time() - start_time
                if elapsed_time > 5:
                    log.dbg(0, lambda: 'elapsed time: %.2f SQL: %s' %
                                       (elapsed_time, repr(cmd)))

                return result

            except Exception as err:
                msg = "Exception: %s CMD: %s" % (repr(err), repr(cmd))
                log.err(msg)
                print("*" * 80)
                trace = traceback.format_exc(10)
                print(trace)
                print("*" * 80)
                return [(msg,)]

            finally:
                if cursor:
                    log.dbg(2, lambda: "Closing cursor")
                    cursor.close()
                self._lock.release()

        else:
            try:
                self._lock.acquire()
                cursor = self.get_cursor()
                cursor.execute(cmd)
                return cursor.fetchall()

            finally:
                if cursor:
                    log.dbg(2, lambda: "Closing cursor")
                    cursor.close()
                self._lock.release()

    def select_one(self, cmd, default, catch=True):

        sql_result = self.select(cmd, catch=catch)
        if sql_result is None:
            return default

        result = default
        for i, item in enumerate(sql_result):
            log.dbg(2, lambda: "item: %s" % repr(item))
            if i == 0:
                result = item[0]
            else:
                log.err("Unexpected result: CMD: %s index: %d" % (result, i))
        return result
