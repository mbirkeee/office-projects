import argparse
import os
import queue
import hashlib

class Application(object):

    """
    ../backup/mak/backup -b . -d /home/mikeb/nobackup/office-projects -l . -f
    """

    def __init__(self):

        self._file_count = 0
        self._count_file_skip = 0
        self._dir_queue = queue.Queue()
        self._have_dict = {}
        self._new_dict = {}

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
                    raise ValueError("unexpcted line")

        finally:
            if f: f.close()

        return

    def run(self):

        self._dir_queue.put_nowait('/home/mikeb/nobackup/office-projects')

        self.read_old_file('2020-10-13.txt')
        self.scan()
        self.compare()

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

        print("Detected %d files" % self._file_count)
        print("Skipped %d files" % self._count_file_skip)

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

            if item.path.endswith('.o'):
                self._count_file_skip += 1
                # print("Skipping file %d: %s" % (self._count_file_skip, item.path))
                continue

            stat = item.stat()
            mtime = stat.st_mtime

            if item.path in self._new_dict:
                raise ValueError("NEW: already know about %s" % item.path)

            md5sum = self.get_file_md5sum(item.path)

            self._new_dict[item.path] = md5sum
            self._file_count += 1


#            print("File %d: %s mtime: %s" %
#                (self._file_count, item.path, repr(mtime)))

    def get_file_md5sum(self, file_name):
        hash_md5 = hashlib.md5()
        with open(file_name, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()


if __name__ == '__main__':

    app = Application()
    app.run()