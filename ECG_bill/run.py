import argparse
import os
import queue
import hashlib
import datetime
import json
import time
import traceback
import sys
import MySQLdb

from pdfminer.pdfinterp import PDFResourceManager, PDFPageInterpreter
from pdfminer.pdfpage import PDFPage
from pdfminer.converter import TextConverter
from pdfminer.layout import LAParams
import io

SKIP_CR_ON_INPUT = False

# from my_sql import MyConnector
MONTH_MAP = {
    'JAN':1, 'FEB':2, 'MAR':3, 'APR': 4, 'MAY':5,  'JUN': 6,
    'JUL':7, 'AUG':8, 'SEP':9, 'OCT':10, 'NOV':11, 'DEC':12
}
class Claim(object):
    def __init__(self):
        self._phn = None
        self._firstName = None
        self._lastName = None
        self._gender = None
        self._birthday = None
        self._acqDate = None
        self._patientID = None

    def __repr__(self):
        result = ''
        result += "%20s" % self._lastName
        result += "%20s" % self._firstName
        result += "%15s" % self._phn
        result += "%4s"  % self._gender
        result += "%20s" % self._birthday
        result += "%25s" % self._acqDate
        return result

                    # print("%20s  %20s %20s" % (lastNames[i], firstNames[i], phns[i]))

    def setPatientID(self, value):
        self._patientID = value

    def setAcqDate(self, value):
        self._acqDate = value

    def getAcqDate(self):
        return self._acqDate

    def setBirthday(self, value):
        self._birthday = value

    def getBirthday(self):
        return self._birthday

    def getBirthdayInt(self):
        parts = self._birthday.split("-")
        # print(parts)
        day = int(parts[0])
        year = int(parts[2].upper())
        month = MONTH_MAP.get(parts[1].upper())
        # print(year, month, day)
        result = year * 10000 + month * 100 + day

        return result

    def setPhn(self, value):
        self._phn = value

    def getPhn(self):
        return self._phn

    def getPhnProv(self):
        return 'SK'

    def setFirstName(self, value):
        self._firstName = value

    def getFirstName(self):
        return self._firstName

    def setLastName(self, value):
        self._lastName = value

    def getLastName(self):
        return self._lastName

    def setGender(self, value):
        self._gender = value

    def getGender(self):
        return self._lastName

    def getGenderInt(self):
        if self._gender == 'M':
            return 0
        elif self._gender == 'F':
            return 1

        raise ValueError("bad gender")

    def getTitle(self):
        if self._gender == 'M':
            return 'Mr.'
        elif self._gender == 'F':
            return 'Ms.'

        raise ValueError("bad gender")
class SQL(object):

    def __init__(self):
        self._db = None

    def connect(self):

        try:
            self._db = MySQLdb.connect(host="127.0.0.1", db="clinic_active", user="mikenp", port=3307)
        except:
            self.db = None

        if self._db is None:
            return False

        return True

    def select(self, cmd):

        print("CMD: %s" % cmd)
        result = None
        c = None

        try:
            c = self._db.cursor()
            c.execute(cmd)
            result =  c.fetchall()

        finally:
            if c: c.close()

        return result

    def select_one(self, cmd):

        print("CMD: %s" % cmd)
        c = None

        try:
            c = self._db.cursor()
            c.execute(cmd)
            result = c.fetchall()

            if len(result) != 1:
                print(result(result))
                raise ValueError("wanted 1, got %d" % len(result))

        finally:
            if c: c.close()

        return result[0][0]

    def execute(self, cmd):
        print("CMD: %s" % cmd)
        c = None
        try:
            c = self._db.cursor()
            c.execute(cmd)

        finally:
            if c: c.close()

class Application(object):

    def __init__(self, arga):

        print(args)
        self._pdfFileName = args.ecg_file
        self._db = None
        self._skip_database = args.skip_database
        self._claims = []
        self._sql = SQL()


    def run(self):

        if not self._skip_database:
            if self._sql.connect() is False:
                print("failed to connect to database")
                print("")
                print("ssh -g -L 3307:localhost:3306 mikeb@drbree.com")
                print("")
                return

        self.readPDFFile()
        self.displayClaims()
        self.checkPatients()

    def checkPatients(self):

        for claim in self._claims:
            self.checkPatient(claim)

    def checkPatient(self, claim):
        # First, see if patient is in the database
        phn = claim.getPhn()
        sql = "select id, first_name, last_name, date_of_birth, gender, not_deleted from patients where health_num=%s" % phn

        rows = self._sql.select(sql)
        print(rows)
        print("LEN ROWS: %d", len(rows))
        rowCount = len(rows)

        if rowCount == 0:
            self.createPatient(claim)
            raise ValueError("temp stop")

        elif rowCount == 1:
            print("need to validate patient")

        else:
            raise ValueError("Got multiple results")

    def my_input(self, msg, echo=True):

        while True:
            if SKIP_CR_ON_INPUT or echo==False:
                print(msg, end='')
                sys.stdout.flush()
                ch = self.get_char()

                # if ord(ch) == 13:
                #     print("\n")
                #     continue

                if echo:
                    print(ch)

                return ch
            else:
                result = input(msg)
                result = result.strip().lower()
                if not result:
                    continue
                return result[0]

    def createPatient(self, claim):
        print("need to create patient %s %s" % (claim.getFirstName(), claim.getLastName()))

        result = self.my_input("create patient [y/n]?")
        print("THIS IS THE RESULT", repr(result))

        # raise ValueError("temp stop")

        try:
            self._sql.execute("lock tables patients write")

            maxId = int(self._sql.select_one("select max(id) from patients"))
            # print("MAX ID: %d" % maxId)
            maxId += 1
            cmd = "insert into patients (id, created) values (%d, now())" % (maxId)
            self._sql.execute(cmd)

            cmd = 'update patients set first_name="%s", last_name="%s", health_num="%s" where id=%d' % \
                (claim.getFirstName(), claim.getLastName(), claim.getPhn(), maxId)

            self._sql.execute(cmd)

            cmd = 'update patients set provider_id=1, gender=%d, title="%s" where id=%d' % \
                (claim.getGenderInt(), claim.getTitle(), maxId)
            self._sql.execute(cmd)

            cmd = 'update patients set not_deleted=1, health_num_prov="%s", date_of_birth=%d where id=%d' % \
                (claim.getPhnProv(), claim.getBirthdayInt(), maxId)
            self._sql.execute(cmd)

            cmd = 'update patients set city="Saskatoon", province="SK", country="Canada" where id=%d' % maxId
            self._sql.execute(cmd)

            claim.setPatientID(maxId)
            # dat of birth
            # PHN prov
            # not deleted
            # gennder
             # provider id
            # date of death
            #
        finally:
            self._sql.execute("unlock tables")

    def displayClaims(self):
        for i, claim in enumerate(self._claims):
            print("Claim: %3d: %s" % (i, repr(claim)))
            bday = claim.getBirthdayInt()
            print("bday: %d" % bday)

    def readPDFFile(self):

        text = self.getTextFromPDF(self._pdfFileName)
        items = text.splitlines(keepends=False)
        items = self.clean(items)

        # for item in items:
        #     print("item", item)

        lastNames   = self.getLastNames(items)
        firstNames  = self.getFirstNames(items)
        phns        = self.getPHNs(items)
        genders     = self.getGenders(items)
        birthdays   = self.getBirthdays(items)
        acqDates    = self.getAcqDates(items)

        count = len(lastNames)
        if len(firstNames) != count:
            raise ValueError("Got %d first names; expected %d" %(len(firstNames), count))

        if len(phns) != count:
            raise ValueError("Got %d PHNs; expected %d" %(len(phns), count))

        if len(genders) != count:
            raise ValueError("Got %d genders; expected %d" %(len(genders), count))

        if len(birthdays) != count:
            raise ValueError("Got %d birthdays; expected %d" %(len(birthdays), count))

        if len(acqDates) != count:
            raise ValueError("Got %d acqDates; expected %d" %(len(acqDates), count))

        for i in range(count):

            claim = Claim()
            claim.setFirstName(firstNames[i])
            claim.setLastName(lastNames[i])
            claim.setPhn(phns[i])
            claim.setGender(genders[i])
            claim.setBirthday(birthdays[i])
            claim.setAcqDate(acqDates[i])

            self._claims.append(claim)

        # print("Found %d last names" % len(lastNames))
        # print("Found %d first names" % len(firstNames))
        # print("Found %d PHNs" % len(phns))


    def getPatientId(self, claim):

        cmd = "select * from patients where phn=%s" % claim.getPhn()
        c = self._db.cursor()
        c.execute(cmd)
        print(cmd)

        while True:
            row = c.fetchone()
            if row is None: break
            print("ROW", row)

    def clean(self, items):
        result = []
        for item in items:
            item = item.strip()
            if len(item) == 0: continue
            result.append(item)

        return result

    def getAcqDates(self, items):
        inList = False

        result = []

        for item in items:
            if inList:
                if item == 'Req. MD':
                    inList=False
                else:
                    # print("Found last Name: '%s'" % item)
                    result.append(item)
            else:
                if item == 'Acquisition Date':
                    inList = True

        return result

    def getBirthdays(self, items):
        inList = False

        result = []

        for item in items:
            if inList:
                if item == 'Acquisition Date':
                    inList=False
                else:
                    # print("Found last Name: '%s'" % item)
                    result.append(item)
            else:
                if item == 'Date of Birth':
                    inList = True

        return result

    def getLastNames(self, items):
        inList = False

        result = []

        for item in items:
            if inList:
                if item == 'First':
                    inList=False
                else:
                    # print("Found last Name: '%s'" % item)
                    result.append(item)
            else:
                if item == 'Last Name':
                    inList = True

        return result

    def getFirstNames(self, items):
        inList = False

        result = []

        for item in items:
            if inList:
                if item == 'HSN':
                    inList=False
                else:
                    # print("Found First Name: '%s'" % item)
                    result.append(item)
            else:
                if item == 'First':
                    inList = True

        return result

    def getPHNs(self, items):
        inList = False

        result = []

        for item in items:
            if inList:
                if item == 'Gender':
                    inList=False
                else:
                    # print("HSN: '%s'" % item)
                    result.append(item)
            else:
                if item == 'HSN':
                    inList = True

        return result

    def getGenders(self, items):
        inList = False

        result = []

        for item in items:
            if inList:
                if item == 'Date of Birth':
                    inList=False
                else:
                    # print("Gender: '%s'" % item)
                    result.append(item)
            else:
                if item == 'Gender':
                    inList = True

        return result

    def getTextFromPDF(self, filename):
        resource_manager = PDFResourceManager()
        fake_file_handle = io.StringIO()
        converter = TextConverter(resource_manager, fake_file_handle, laparams=LAParams())
        page_interpreter = PDFPageInterpreter(resource_manager, converter)

        with open(filename, 'rb') as fh:
            for page in PDFPage.get_pages(fh, caching=True, check_extractable=True):
                page_interpreter.process_page(page)

            text = fake_file_handle.getvalue()

        # close open handles
        converter.close()
        fake_file_handle.close()
        return text

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='ECG Billing')
    parser.add_argument('-f', '--ecg-file', help='ECG File (PDF)', required=True)
    parser.add_argument('-s', '--skip-database', help='Skip Database Connection', required=False, action='store_true')
#    parser.add_argument('-b', '--directory-meta', help='Meta directory', required=True)
    args = parser.parse_args()

    app = Application(args)
    app.run()