import argparse
import random
import simplejson


class Runner(object):
    """

    """
    def __init__(self, args):

        try:
            self._filename = args.file
        except:
            self._filename = None

        self._pid_dict = {}
        self._fd_dict = {}
        self._port_dict = {}


    def run(self):

        print "filename"
        print self._filename

        for line in open(self._filename, "r"):
            # print "LINE:", line.strip()

            x = simplejson.loads(line.strip())

            l = x.get("l")
            # print "L: '%s'" % l
            # print "GOT X", x

            if l.startswith( "pid:"):
                self.process_pid(l)

            elif l.startswith( "tcp:"):
                self.process_tcp(l)

            elif l.startswith( "udp:"):
                # print "this is a UDP line"
                pass
            else:
                # print "IGNORE LINE", l
                pass


        # print "PID DATA"
        #
        # for pid, data in self._pid_dict.iteritems():
        #     print "PID", pid, data.get("cmdline", "NO CMD"), data.get("cwd", "NO CWD")
        #
        # print "FD DATA"
        #
        # for fd, pid_list in self._fd_dict.iteritems():
        #     print "FD: %d PID_LIST: %s" % (fd, repr(pid_list))

        for port, fd_list in self._port_dict.iteritems():
            total_pids = []
            print "PORT", port

            for fd in fd_list:
                pid_list = self._fd_dict.get(fd, [])
                total_pids.extend(pid_list)

            total_pids = list(set(total_pids))

            # print "  FD:", fd, pid_list
            for pid in total_pids:
                pid_data = self._pid_dict.get(pid, {})
                cmd = pid_data.get('cmdline', "NO CMD")
                cwd = pid_data.get('cwd', "NO CWD")
                print "    PID: %d CMD: %s CWD: %s" % (pid, cmd, cwd)


    def process_tcp(self, line):
        parts = line.split()
        # print "NUM TCP PARTS", len(parts), line
        # print parts


        # print line

        local_addr = parts[2]
        inode= parts[10]

        try:
            inode = int(inode)
        except:
            print "invalid"
            return

        # print local_addr, inode

        addr_parts = local_addr.split(":")

        # print addr_parts
        port = addr_parts[1]

        fd_list = self._port_dict.get(port, [])
        fd_list.append(inode)
        self._port_dict[port] = fd_list


    def process_pid(self, line):
        # print "process PID line: %s" % line

        parts = line.split(" ")
        # print "GOT %d parts" % len(parts)

        pid = int(parts[1])
        # print "THIS IS THE PID", pid

        pid_data = self._pid_dict.get(pid, {})

        # print "PARTS[2]", parts[2]
        pid_thing = parts[2]

        update_pid_dict = True
        if pid_thing.startswith("cmdline"):

            if pid_data.has_key("cmdline"):
                raise ValueError("this pid already has a command line!!!")

            # print "GOT COMMAND LINE", parts[3:]
            command_line = " ".join(parts[3:])
            command_line = command_line.strip()
            # print "COOMMAND LINE: '%s'" % command_line

            pid_data['cmdline'] = command_line

        elif pid_thing.startswith("cwd"):

            if pid_data.has_key("cwd"):
                raise ValueError("this pid already has a command line!!!")

            cwd = " ".join(parts[3:])
            cwd = cwd.strip()
            pid_data['cwd'] = cwd

        elif pid_thing.startswith("socket"):
            update_pid_dict = False
            # print "GOT A SOCKET FD", pid_thing
            fd = ''
            for c in pid_thing:
                if c < '0' or c > '9': continue
                fd += c

            fd = int(fd)
            # print "THIS IS THE FD", fd

            pid_list = self._fd_dict.get(fd, [])
            pid_list.append(pid)
            self._fd_dict[fd] = pid_list

        if update_pid_dict:
            self._pid_dict[pid] = pid_data


if __name__ == "__main__":

   parser = argparse.ArgumentParser(description='Test procread stuff')
   parser.add_argument("-f", "--file", help="input file")

   args = parser.parse_args()

   runner = Runner(args)
   runner.run()
