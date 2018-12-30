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



    def run(self):

        print "filename"
        print self._filename

        for line in open(self._filename, "r"):
            print "LINE:", line.strip()

            x = simplejson.loads(line.strip())

            # print "GOT X", x

if __name__ == "__main__":

   parser = argparse.ArgumentParser(description='Test procread stuff')
   parser.add_argument("-f", "--file", help="input file")

   args = parser.parse_args()

   runner = Runner(args)
   runner.run()
