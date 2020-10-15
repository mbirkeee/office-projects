import argparse
import os
import queue
import hashlib
import datetime
import json
import time

SAVE_FILE_PATTERN   = '_####_##_##-##_##_##.'


class FileSaver(object):

    def __init__(self, path=None, prefix='FileSaver', keep_last=5):

        self.path = path
        #     # This exception was used to find any places in pvmon/iocmon
        #     # that were hardcoding the paths.
        #     # raise ValueError("File saver called with a path specified: %s" % path)
        # else:
        #     hostname = socket.gethostname().lower()
        #     hostname = hostname.replace('.clsi.ca', '')
        #     self.path = os.path.join(os.curdir, "data", hostname, "json")
        # self.dbl = 1

        self.prefix = prefix
        self.keep_last = keep_last

        print("FILE_SAVER: PATH:   '%s'" % self.path)
        print("FILE_SAVER: PREFIX: '%s'" % self.prefix)

    def save(self, json_string):

        t = datetime.datetime.now()

        filename = '%s_%4d_%02d_%02d-%02d_%02d_%02d.json' % \
                   (self.prefix, t.year, t.month, t.day, t.hour, t.minute, t.second)

        path = os.path.join(self.path, filename)

        print("path: %s" % path)

        f = open(path, 'w')
        f.write(json_string)
        f.close()
        self.purge()

    def get_newest(self):

        files = self.get_matching_files()

        if len(files) == 0:
            return

        return os.path.join(self.path, files[0])

    def purge(self):
        files = self.get_matching_files()

        purge = files[self.keep_last:]
        for file in purge:
            print("purge: %s" % file)
            os.remove(os.path.join(self.path, file))

    def get_matching_files(self):

        print("path: %s prefix: %s" % (self.path ,self.prefix))

        files = [f for f in os.listdir(self.path)]

        matching = []
        for name in files:
            file = os.path.join(self.path, name)
            if not os.path.isfile(file):
                continue

            if not name.startswith("%s_" % self.prefix):
                continue

            if not name.endswith(".json"):
                continue

            matched = False
            p = len(self.prefix)

            print("Consider file: %s" % repr(name))

            for i, c in enumerate(name[p:]):

                try:
                    expect = SAVE_FILE_PATTERN[i]

                except Exception as err:
                    print("exception parsing file name %s: %s" % (name, repr(err)))
                    break

                # print "Consider char: %d: got: %c expect: %c" % (i, c, expect)

                if expect == '#' and c.isdigit():
                    continue

                elif expect == '.' and c == expect:
                    matched = True
                    break

                elif c == expect:
                    continue

            if not matched:
                # print "File '%s' does not match pattern" % name
                continue

            # print "this file matches!!!"
            matching.append(name)

        # print "Matching files", matching

        matching = sorted(matching)
        return [i for i in reversed(matching)]


class Application(object):

    """
    ../backup/mak/backup -b . -d /home/mikeb/nobackup/office-projects -l . -f
    """

    def __init__(self, arga):

        print(args)

        self._directory_meta = args.directory_meta
        self._dir_queue = queue.Queue()
        self._have_dict = {}
        self._new_dict = {}
        self._md5sum_dict = None

        self._count_cache_hit = 0
        self._count_file_skipped = 0
        self._count_file_total = 0
        self._count_file_changed = 0
        self._count_file_new = 0
        self._count_file_included = 0
        self._count_file_md5sum = 0

        self._count_progress = 0

        self._file_saver = FileSaver(path=self._directory_meta, prefix="md5sum_cache")

        paths = list(set(args.directory_backup))
        for item in paths:
            self._dir_queue.put_nowait(item)

    def read_old_file(self, file_name):

        f = None

        try:
            f = open(file_name, 'r')

            for line in f:
                line = line.strip()
                # print('LINE: %s' % line)

                if line.startswith('P:'):
                    path = line[2:]
                    # print("PATH: %s" % path)

                elif line.startswith('F:'):
                    cksum = line[2:34]
                    name = line[35:]
                    # print("CRC: >%s< NAME: >%s<" % (cksum, name))

                    total = os.path.join(path, name)
                    # print("TOTAL: %s" % total)

                    if total in self._have_dict:
                        raise ValueError("OLD: Already know about %s" % total)

                    self._have_dict[total] = cksum

                elif line.startswith('E:'):
                    break

                else:
                    raise ValueError("unexepcted line")

        finally:
            if f: f.close()

        return

    def run(self):

        self.md5sum_cache_read()

        # self._dir_queue.put_nowait('/home/mikeb/nobackup/office-projects')

        # self.read_old_file('2020-10-13.txt')
        self.scan()
        self.md5sum_cache_save()
        self.print_stats()

    def print_stats(self):
        print("---------------------------------------")
        print("Files examined: %d" % self._count_file_total)
        print("Files new: %d" % self._count_file_new)
        print("Files changed: %d" % self._count_file_changed)
        print("Files skipped: %d" % self._count_file_skipped)
        print("Files included: %d" % self._count_file_included)
        print("md5sum cache hits: %d" % self._count_cache_hit)
        print("md5sum computations: %d" % self._count_file_md5sum)

        # self.compare()

    def md5sum_cache_save(self):

        start_time = time.time()
        data_str = json.dumps(self._md5sum_dict)
        self._file_saver.save(data_str)
        elapsed_time = time.time() - start_time
        print("Time to write cache: %.1f" % elapsed_time)

    def md5sum_cache_read(self):

        cache_file = self._file_saver.get_newest()
        if cache_file is None:
            self._md5sum_dict = {}
        else:
            f = None
            try:
                f = open(cache_file, 'r')
                data = json.load(f)
                self._md5sum_dict = data
            finally:
                if f: f.close()

    def compare(self):

        for k, v in self._have_dict.items():
            if not k in self._new_dict:
                print("OLD FILE NOT IN NEW: %s" % k)
            else:
                v2 = self._new_dict.get(k)
                if v != v2:
                    print("MD5SUM mismatch: %s" % k)

        for k, v in self._new_dict.items():
            if not k in self._have_dict:
                print("NEW FILE NOT IN OLD: %s" % k)
            else:
                v2 = self._have_dict.get(k)
                if v != v2:
                    print("MD5SUM mismatch: %s" % k)

    def scan(self):

        while True:

            try:
                item = self._dir_queue.get(block=False)

            except queue.Empty:

                break

            if item is None:
                break

            self.process_directory(item)

    def process_directory(self, directory):

        for item in os.scandir(directory):

            # print( repr(item))
            # print( dir(item))
            # print( "is dir: ", item.is_dir())

            if item.is_symlink():
                print("SYMLINK: %s FILE: %s DIR: %s" % (item.path, item.is_file(), item.is_dir()))

            if item.is_dir():
                self._dir_queue.put_nowait(item.path)
                continue

#             print( "is file: ", item.is_file())
#             print( "is symlink: ", item.is_symlink())
#             print( "name: ", item.name)
#             print( "path: ", item.path)
#             print( "stat", repr(item.stat()))

            self._count_file_total += 1

            self._count_progress += 1
            if self._count_progress >= 10000:
                self.print_stats()
                self._count_progress = 0
                self.md5sum_cache_save()

            if item.path.endswith('.o'):
                self._count_file_skipped += 1
                # print("Skipping file %d: %s" % (self._count_file_skip, item.path))
                continue

            stat = item.stat()
            mtime = stat.st_mtime

            if item.path in self._new_dict:
                raise ValueError("NEW: already know about %s" % item.path)

            # See if the md5dum is cached
            cached = self._md5sum_dict.get(item.path)
            if cached:
                have_mtime = cached.get('t', 0)
                if have_mtime == mtime:
                    self._count_cache_hit += 1
                else:
                # This file must have changed
                    self._count_file_changed += 1
                    print("FILE CHANGED: %s" % item.path)
                    cached = None

            else:
                self._count_file_new += 1

            if cached:
                # print("CACHE HIT: %s" % item.path)
                md5sum = cached['s']
            else:
                #  print("Get MD5SUM FOR: %s" % item.path)
                self._count_file_md5sum += 1
                md5sum = self.get_file_md5sum(item.path)
                self._md5sum_dict[item.path] = {'s': md5sum, 't': mtime}

            self._count_file_included += 1
            self._new_dict[item.path] = md5sum


#            print("File %d: %s mtime: %s" %
#                (self._file_count, item.path, repr(mtime)))

    def get_file_md5sum(self, file_name):
        hash_md5 = hashlib.md5()
        with open(file_name, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Backup Specified directories')
    parser.add_argument('-d', '--directory-backup', help='Directory to backup', required=True, action='append')
    parser.add_argument('-b', '--directory-meta', help='Meta directory', required=True)
    args = parser.parse_args()

    app = Application(args)
    app.run()