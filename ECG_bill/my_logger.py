import traceback
import datetime

class MyLogger(object):

    def __init__(self, debug_level=None):
        self._debug_level = debug_level
        self._call_level = 1

    def emit(self, label, arg):
        file, function, line = self.get_details()

        # t = "{:%H:%M:%S:%f}".format(datetime.datetime.now())
        # Add date to logs
        t = "{:%m-%d %H:%M:%S:%f}".format(datetime.datetime.now())

        if isinstance(arg, str):
            print("%s: %s %s (%s:%d): %s" %    (t, label, function, file, line, arg))
        elif callable(arg):
            print("%s: %s %s (%s:%d): %s" %    (t, label, function, file, line, arg()))
        else:
            print("%s: %s %s (%s:%d): logger got -> type(arg): %s" %  (t, label, function, file, line, type(arg)))

    def dbg(self, level, arg):
        if level <= self._debug_level:
            self.emit("DBG(%d) " % level, arg)

    def log(self, arg=None):
        self.emit("LOG(1) ", arg)

    def info(self, arg=None):
        self.emit("LOG(1) ", arg)

    def error(self, arg=None):
        self.emit("ERR(1) ", arg)

    def err(self, arg=None):
        self.emit("ERR(1) ", arg)

    def warn(self, arg=None):
        self.emit("WARN(1)", arg)

    def call(self, arg="called"):
        self.emit("CALL(1)", arg)

    def get_details(self):

        # for item in x:
        #     print item

        try:
            x = traceback.extract_stack()

            source_item = x[-4]
            # print "This is the source item", source_item

            file = source_item[0]
            line = source_item[1]
            function = source_item[2]

        except Exception as err:
            print("*"*80)
            print("*"*80)
            print("my_logger.get_details() failed:", repr(err))
            print("*"*80)
            print("*"*80)

            file = 'unknown'
            function = 'unknown'
            line = 0

        return (file, function, line)

