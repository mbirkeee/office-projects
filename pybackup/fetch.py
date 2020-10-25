import argparse
import queue
import os
import datetime
import subprocess
import time
import hashlib
import shutil

# CMD_SCP = "/usr/bin/scp -p -B -C -q -l 8192 -v -o ConnectTimeout=30"

CMD_GZIP    = "/bin/gzip"
CMD_SCP     = "/usr/bin/scp"

SCP_ARGS = [
     '-p',
     '-B',
     '-C',
#     '-q ',   # Wont run under subprocess with this opt
     '-l 8192',
#     '-v',
     '-o ConnectTimeout=30',
]

class Done(Exception):
    pass

class Application(object):

    """
    ../backup/mak/backup -b . -d /home/mikeb/nobackup/office-projects -l . -f

    /home/mikeb/bin/backup -h server@drbree.com  \
         -r /data/backups/local_xcelera \
         -d /data3/backups/hazel3_xcelera \
         -l /data3/backups/hazel3_xcelera/logs > /tmp/backup_hazel3_xcelera_$DATE


    """

    def __init__(self, args):
        print(args)

        self._directory_remote = args.directory_remote
        self._directory_local = args.directory_local
        self._directory_log = args.directory_log
        self._host = args.host

        self._dict_remote_dirs = {}
        self._dict_have = {}
        self._dict_want = {}

        self._count_progress = 0
        self._dir_queue = queue.Queue()

        self._count_got = 0

    def run(self):

        try:
            self.get_remote_list()
            self.scan_local()
            self.print_stats()

            self.clean_temp()
            self.get_files()

        except Done as d:
            print(d)
            print(dir(d))
            self.log(d.msg)

        self.print_stats()
        self.log("Done")

    def clean_temp(self):

        temp_dir = os.path.join(self._directory_local, 'temp')
        items = os.scandir(temp_dir)
        for item in items:
            self.log("old temp file: %s" % item.path)
            try:
                os.unlink(item.path)
            except Exception as err:
                self.log("Failed to remove old temp file: %s %s" % (item.path, err))

    def get_files(self):

        self._count_got = 0

        for file, md5sum in self._dict_want.items():

            if md5sum in self._dict_have:
                # self.log("Already have: %s" % file)
                continue

            # self.log("Want: %s" % file)
            target = os.path.join(self._directory_local, "temp", md5sum)

            # Get the file in the temp dir
            if not self.scp_file(file, target):
                self.log("Failed: %s" % file)
                continue

            # Get the md5sum of the file just transferred
            md5sum_new = self.get_file_md5sum(target)
            if md5sum_new != md5sum:
                # This could happen if the file was being updated while the
                # remote backup list was being made.  So for now we just
                # skip it
                self.log("Error: ms5sum mis-match for: %s" % target)
                continue

            # gzip the file
            cmd = [CMD_GZIP, target]
            cmd_str = " ".join(cmd)

            result = subprocess.run(cmd, capture_output=True)

            err_str =  result.stderr.decode('utf-8')
            err_str = err_str.strip()
            if err_str:
                self.log("Error: %s" % err_str)

            if result.returncode != 0:
                self.log("Failed: cmd: %s" % cmd_str)
                continue

            source = target + '.gz'

            stat_src = os.stat(source)

            # print("This is the md5sum", md5sum)

            # Now we must make the subdirectory for the newly acquired file
            dir1 = md5sum[0:2]
            dir2 = md5sum[2:4]

            path = os.path.join(self._directory_local, "files", dir1, dir2)
            # print("must make directory: %s" % path)

            try:
                os.makedirs(path)
            except Exception as err:
                # print("exception making directory: %s" % err)
                pass

            target = os.path.join(path, "%s.gz" % md5sum)

            # print("SRC", source)
            # print("TGT", target)

            shutil.copy2(source, target)

            stat_tgt = os.stat(source)

            # print("stat_src", stat_src)
            # print("stat_tgt", stat_tgt)

            self._dict_have[md5sum] = True

    def get_file_md5sum(self, file_name):
        hash_md5 = hashlib.md5()
        with open(file_name, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()


    def log(self, msg):
        n = datetime.datetime.now()
        time_str = n.strftime("%H:%M:%S")
        print("%s %s" % (time_str, msg.strip()))

    def get_remote_list(self):
        # First check if we already have the remote file
        n = datetime.datetime.now()
        file_name = n.strftime("%Y-%m-%d.txt")
        file_name_compressed = file_name + ".gz"

        file_name_finished = os.path.join(self._directory_local, "done", file_name_compressed)
        if os.path.exists("File %s already processed" % file_name_finished):
            raise Done("File %s already processed" % file_name_finished)

        file_name_target = os.path.join(self._directory_local, "new", file_name)

        if os.path.exists(file_name_target):
            self.log("Already have: %s" % file_name_target)
            self.process_remote_list(file_name_target)
            return

        file_name_src = os.path.join(self._directory_remote, file_name_compressed)
        file_name_tgt = os.path.join(self._directory_local, "new", file_name_compressed)

        self.log("Fetching: %s" % file_name_src)

        if not self.scp_file( file_name_src, file_name_tgt):
            raise Done("Failed to get: %s" % file_name_src)

        cmd = [CMD_GZIP, '-d', file_name_tgt]
        cmd_str = " ".join(cmd)

        result = subprocess.run(cmd, capture_output=True)

        err_str =  result.stderr.decode('utf-8')
        err_str = err_str.strip()
        if err_str:
            self.log("err: %s" % err_str)

        if result.returncode != 0:
            self.log("cmd: %s" % cmd_str)
            raise Done("%s failed" % cmd_str)

        self.process_remote_list(file_name_target)
        return

    def process_remote_list(self, file_name):
        """
        Read in the remote list of files
        """

        directory = None
        try:
            f = open(file_name)
            for line in f:
                line = line.strip()
                if line.startswith("P:"):
                    directory = line[2:]
                    if directory in self._dict_remote_dirs:
                        raise ValueError("Already know about directory: %s" % directory)

                    self._dict_remote_dirs[directory] = True

                elif line.startswith('F:'):
                    md5sum = line[2:34]
                    file_name = line[35:]
                    # print("line", line)
                    # print("md5sum: >%s<" % md5sum)
                    # print("file_name: >%s<" % file_name)

                    total_name = os.path.join(directory, file_name)
                    if total_name in self._dict_want:
                        raise ValueError("Already know about file: %s" % total_name)

                    self._dict_want[total_name] = md5sum

                elif line.startswith('E:'):
                    break
                else:
                    raise ValueError("dont know how to process line")
                # print("Process line: %s" % line)
                #
                # if line

        finally:
            if f: f.close()


    def scp_file(self, src, tgt):
        # cmd = "%s %s:%s %s" % (CMD_SCP, self._host, file_name_src, file_name_tgt)

        start_time = time.time()

        cmd = [CMD_SCP]
        cmd.extend(SCP_ARGS)
        cmd.append("%s:%s" % (self._host, src))
        cmd.append(tgt)

        cmd_str = " ".join(cmd)

        result = subprocess.run(cmd, capture_output=True)

        # print("result", result)
        # print("CMD: %s" % cmd)

        err_str =  result.stderr.decode('utf-8')
        err_str = err_str.strip()

        if err_str:
            self.log("err: %s" % err_str)

        if result.returncode != 0:
            self.log("cmd: %s" % cmd_str)
            return False

        elapsed_time = time.time() - start_time
        stat = os.stat(tgt)
        # print(stat)
        bytes = stat.st_size
        try:
            bytes_per_sec = bytes / elapsed_time
        except:
            bytes_per_sec = 0

        self._count_got += 1

        self.log("Got %d: %s (%.1f s; %.1f bytes/s)" % (self._count_got, src, elapsed_time, bytes_per_sec))
        return True

        # print("returnCode: %d" % result.returncode)
        # print("stdout: %s" % result.stdout)
        # print("stderr: %s" % result.stderr)

    def print_stats(self):

        self.log("Remote directories: %d" % len(self._dict_remote_dirs))
        self.log("Remote files: %d" % len(self._dict_want))
        self.log("Local files: %d" % len(self._dict_have))

    def scan_local(self):

        have_path = os.path.join(self._directory_local, "files")
        self._dir_queue.put_nowait(have_path)

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

            self._count_progress += 1
            if self._count_progress >= 1000:
                # self.print_stats()
                self.log("Scanned %s local files" % len(self._dict_have))
                self._count_progress = 0
                # self.md5sum_cache_save()

            basename = os.path.basename(item.path)
            parts = basename.split('.')
            md5sum = parts[0]

            # print("Basename: %s" % basename)
            # print("File: %s" % item.path)
            if md5sum in self._dict_have:
                print("Basename: %s" % basename)
                print("File: %s" % item.path)
                raise ValueError("Already have md5sum: %s" % md5sum)

            self._dict_have[md5sum] = True

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Fetch backup files')
    parser.add_argument('-r', '--directory-remote', help='Remote meta directory ', required=True)
    parser.add_argument('-d', '--directory-local',  help='Local directory', required=True)
    parser.add_argument('-l', '--directory-log',    help='Log directory', required=True)
    parser.add_argument('-s', '--host',             help='Host', required=True)
    args = parser.parse_args()

    app = Application(args)
    app.run()