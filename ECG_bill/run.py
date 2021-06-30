import argparse
import sys
import io
import readline

from pdfminer.pdfinterp import PDFResourceManager, PDFPageInterpreter
from pdfminer.pdfpage import PDFPage
from pdfminer.converter import TextConverter
from pdfminer.layout import LAParams

from claim import Claim
from my_sql import SQL

from definitions import SKIP_CR_ON_INPUT

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

        msg = "Next step: Check patients. Continue? [y|n]: "
        result = self.my_input(msg)
        if not self.checkResult(result):
            return

        self.checkPatients()

        msg = "Next step: Create ECG claims. Continue? [y|n]: "
        result = self.my_input(msg)
        if not self.checkResult(result):
            return

        self.createClaims()

    def createClaims(self):

        for claim in self._claims:
            print(claim)
            self.createOneClaim(claim)

    def createOneClaim(self, claim):

        patient_id = claim.getPatientID()
        if patient_id is None:
            patient_id = -1000

        acqDateInt = claim.getAcqDateInt()
        acqTime = claim.getAcqTime()

        # Check if claim exists
        cmd = 'select id from claims where patient_id=%d and fee_code="031D" ' \
            'and service_day=%d and comment_in="%s"' % (patient_id, acqDateInt, acqTime)

        claim_id = self._sql.select_one(cmd, display=False)
        if claim_id is not None:
            msg = "EGC claim id for -- %s, %s' (%s) -- %s -- already created (ID: %d)" % \
                (claim.getLastName(), claim.getFirstName(),
                 claim.getGender(), claim.getAcqDate(), int(claim_id) )
            print(msg)
            # msg = "EGC claim id %s already created for (%s, %s) on %s   Press any key to continue..." % \
            #     (int(claim_id), claim.getLastName(), claim.getFirstName(), claim.getAcqDate() )
            # input(msg)
            return

        doctor_id = self.getReferringDoctorNumbers(claim)
        if doctor_id is None:
            print("No referring doctor selected; ECG claim for -- %s, %s (%s) -- NOT created!" %
                  (claim.getLastName(), claim.getFirstName(), claim.getGender()))
            return

        # print("Patient ID: %d date: %d time: %s" % (patient_id, acqDateInt, acqTime))

        print("----- CREATING ECG CLAIM -----")
        print("Patient:         %s, %s (%s)" % (claim.getLastName(), claim.getFirstName(), claim.getGender()))
        print("Date of birth:   %s (%d)" % (claim.getBirthday(), claim.getBirthdayInt()))
        print("Date of service: %s (%d)" % (claim.getAcqDate(), claim.getAcgDateInt()))
        print("Comment to MSP:  %s" % claim.getAcqTime())
        print("Referring Dr.    %s, %s (Billing: %d)" % (
            claim.getDoctorLastName(), claim.getDoctorFirstName(), claim.getDoctorNumber()))
        # Proceed with claim creation - must get the referring doctor

        result = self.my_input("proceed with claim creation? [y|n]")

    def validateDoctor(self, doctor_number, claim):
        cmd = "select first_name, last_name, id, " \
              "on_msp_list from doctors where doctor_number=%d" % doctor_number

        rows = self._sql.select(cmd)

        if len(rows) != 1:
            print("Doctor with MSP number %d not found in database" % doctor_number)
            return None

        row = rows[0]
        first_name = row[0]
        last_name = row[1]
        doctor_id = int(row[2])
        on_msp_list = int(row[3])
        if on_msp_list != 1:
            raise ValueError("doctor not on msp list")

        msg = "You selected -- Dr. %s, %s -- (MSP: %d ID: %d) -- Is this correct? [y|n]: " % \
              (last_name, first_name, doctor_number, doctor_id)

        result = self.my_input(msg)
        if result == 'y':
            claim.setDoctorLastName(last_name)
            claim.setDoctorFirstName(first_name)
            claim.setDoctorID(doctor_id)
            claim.setDoctorNumber(doctor_number)
            return doctor_id

        return None

    def getReferringDoctorNumbers(self, claim):

        last_name = claim.getRefDrLastName()
        # print("%s ----> '%s'" % (claim.getRefDr(), last_name))
        if last_name:
            cmd  = 'select id, first_name, doctor_number, on_msp_list from doctors ' \
                 + 'where last_name="%s" ' % last_name \
                 + 'and doctor_number>0 and on_msp_list=1 and not_deleted=1'

            rows = self._sql.select(cmd)

            # print("DR RESULT", repr(result))
            # print("LEN esult: %d" % len(result))

            if len(rows) == 0:
                print("Doctor %s not found in database (%s)" % last_name, claim.getRefDr() )
                last_name = None

            elif len(rows) == 1:
                row = rows[0]
                doctor_number = int(row[2])
                doctor_id = self.validateDoctor(doctor_number, claim)
                if doctor_id is not None:
                    return doctor_id
            else:
                # loop until selection made
                while True:
                    print("Select doctor for claim -- %s, %s (%s) -- (Ref: %s)" %
                          (claim.getLastName(), claim.getFirstName(),
                           claim.getGender(), claim.getRefDr() ))

                    for i, row in enumerate(rows):
                        first_name = row[1]
                        billing_number = row[2]
                        print("%2d: Dr. %s, %s (%s)" % (i+1, last_name, first_name, billing_number))

                    selection = input("Select 1-%d [n=None]: " % len(rows))
                    if not selection: continue
                    selection = selection.strip().lower()
                    if selection == 'n': break

                    try:
                        index = int(selection)
                    except Exception:
                        continue

                    if index < 1 or index > len(rows):
                        continue

                    row = rows[index-1]
                    doctor_number = int(row[2])
                    doctor_id = self.validateDoctor(doctor_number, claim)
                    if doctor_id is not None:
                        return doctor_id

        if last_name is None:
            msg = "NO DOCTOR - MUST PROMPT for doctor_number"
            result = self.my_input((msg))

        return None,None

    def checkResult(self, result):
        result = result.strip().lower()
        if result.startswith('y'):
            return True
        return False

    def checkPatients(self):

        for i, claim in enumerate(self._claims):
            self.checkPatient(claim)
            if i > 5:
                print("temp early break!!!!")
                break

    def checkPatient(self, claim):

        # First, see if patient is in the database
        phn = claim.getPhn()

        if phn is None:
            print("Skipping patient -- %s, %s (%s) -- with PHN: None" %
                  (claim.getLastName(), claim.getFirstName(), claim.getGender() ))
            return

        phn_prov = claim.getPhnProv()

        # Must search patient database by PHN which is the unique patient identifier
        sql = 'select id, first_name, last_name, date_of_birth, gender, not_deleted '\
             'from patients where health_num="%d" and health_num_prov="%s"' % \
            (phn, phn_prov)

        rows = self._sql.select(sql)
        rowCount = len(rows)

        if rowCount == 0:
            self.createPatient(claim)

        elif rowCount == 1:
            row = rows[0]
            patientID = int(row[0])
            claim.setPatientID(patientID)
            self.validatePatient(claim, row)

        else:
            raise ValueError("Got multiple results")

    def validatePatient(self, claim, row):
        patientID = int(row[0])
        print("Found existing patient -- %s, %s (%s) -- ID: %d" %
            (claim.getLastName(), claim.getFirstName(), claim.getGender(), patientID))

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

    def input_with_prefill(self, prompt, text):

        if text is None:
            text = ''

        def hook():
            readline.insert_text(text)
            readline.redisplay()
        readline.set_pre_input_hook(hook)
        result = input(prompt)
        readline.set_pre_input_hook()
        return result


    def createPatient(self, claim):

        while True:
            msg = "Create Patient (%s, %s) [ y=yes | s=skip | l = last | f = first ]: " % \
                  (claim.getLastName(), claim.getFirstName())

            result = self.my_input(msg)

            if result.startswith('s'):
                print("Skipping patient")
                return

            elif result.startswith('y'):
                break

            elif result.startswith('f'):
                new = self.input_with_prefill("First name: ", claim.getFirstName())
                if new:
                    claim.setFirstName(new, skip_validate=True)

            elif result.startswith('l'):
                new = self.input_with_prefill("Last name: ", claim.getFirstName())
                if new:
                    claim.setLastName(new, skip_validate=True)
        #----------------------------------------------------------------------

         # raise ValueError("temp stop")

        try:
            self._sql.execute("lock tables patients write")

            maxId = int(self._sql.select_one("select max(id) from patients"))
            # print("MAX ID: %d" % maxId)
            maxId += 1
            cmd = "insert into patients (id, created) values (%d, now())" % (maxId)
            self._sql.execute(cmd)

            cmd = 'update patients set first_name="%s", last_name="%s" where id=%d' % \
                (claim.getFirstName(), claim.getLastName(), maxId)
            self._sql.execute(cmd)

            if claim.getPhn() is not None:
                cmd = 'update patients set health_num="%d", health_num_prov="%s" where id=%d' % \
                (claim.getPhn(), claim.getPhnProv(), maxId)
                self._sql.execute(cmd)

            cmd = 'update patients set provider_id=1, gender=%d, title="%s" where id=%d' % \
                (claim.getGenderInt(), claim.getTitle(), maxId)
            self._sql.execute(cmd)

            cmd = 'update patients set not_deleted=1, date_of_birth=%d where id=%d' % \
                (claim.getBirthdayInt(), maxId)
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
            # print("bday: %d" % bday)

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
        refDrs      = self.getRefDrs(items)

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

        if len(refDrs) != count:
            raise ValueError("Got %d refDrs; expected %d" %(len(refDrs), count))

        for i in range(count):

            claim = Claim()
            claim.setFirstName(firstNames[i])
            claim.setLastName(lastNames[i])
            claim.setPhn(phns[i])
            claim.setGender(genders[i])
            claim.setBirthday(birthdays[i])
            claim.setAcqDate(acqDates[i])
            claim.setRefDr(refDrs[i])
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

    def isInt(self, item):
        try:
            i = int(item)
            print("foud an int")
            return True
        except:
            return False

    def getRefDrs(self, items):
        inList = False

        result = []

        for item in items:
            print("ITEM: %s" % item)
            if inList:

                if self.isInt(item):
                    inList = False

                elif item == 'Totals for MD':
                    inList=False

                else:
                    # print("Found Req Dr: '%s'" % item)
                    result.append(item)
            else:
                if item == 'Req. MD':
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
    parser.add_argument('-c', '--create-claim', help='Create Claim', required=False, action='store_true')
#    parser.add_argument('-b', '--directory-meta', help='Meta directory', required=True)
    args = parser.parse_args()

    app = Application(args)
    app.run()
    print("Done")