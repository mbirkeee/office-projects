from definitions import MONTH_MAP

class Claim(object):
    def __init__(self):
        self._phn = None
        self._firstName = None
        self._lastName = None
        self._gender = None
        self._birthday = None
        self._acqDate = None
        self._patient_id = None
        self._refDr = None
        self._acqDateInt = None
        self._acqTime = None
        self._refDrLastName = None
        self._doctor_id = None
        self._doctor_number = None
        self._doctor_last_name = None
        self._doctor_first_name = None

    def __repr__(self):
        result = ''
        result += "%20s " % self._lastName
        result += "%20s " % self._firstName
        result += "%15s " % self._phn
        result += "%4s "  % self._gender
        result += "%20s " % self._birthday
        result += "%25s " % self._acqDate
        result += "%40s" % self._refDr
        return result

                    # print("%20s  %20s %20s" % (lastNames[i], firstNames[i], phns[i]))

    def setRefDr(self, value):
        self._refDr = value

    def getRefDr(self):
        return self._refDr

    def getRefDrLastName(self):

        def checkOnePart(name):
            # print("check 1 part: %s" % name)
            result = self.capitalizeName(name)
            return result

        result = None
        parts = self._refDr.split(',')
        # print("dr parts: %s" % repr(parts))

        # Look for no doctor
        pos = self._refDr.find(' Protocol')
        if pos > 0:
            # print("No doctor!!!")
            return None

        pos = self._refDr.find('Bree')
        if pos >= 0:
            # print("No doctor!!!")
            return None

        if len(parts) >= 1:
            result = checkOnePart(parts[0].strip())

        # if len(parts) >= 2:
        #     # Lets assume this is last,first
        #     last_name = parts[0].strip()
        #     result = checkOnePart()
        #     # parts = last_name.split(' ')
        #     # if len(parts) > 1:
        #     #     x = parts[0].strip().lower()
        #     #     if x == 'dr.':
        #     #         print("got Dr X", repr(parts))
        #     #     else:
        #     #         raise ValueError("not sure hats going on here!!!")
        #     # else:
        #     #     result = self.capitalizeName(last_name)

        # print("returning ref dr last name '%s'" % result)
        return result

    def setPatientID(self, value):
        self._patient_id = int(value)

    def getPatientID(self):
        return self._patient_id

    def setDoctorID(self, value):
        self._doctor_id = int(value)

    def getDoctorID(self):
        return self._doctor_id

    def setDoctorNumber(self, value):
        self._doctor_number = int(value)

    def getDoctorNumber(self):
        return self._doctor_number

    def setDoctorFirstName(self, value):
        self._doctor_first_name = value

    def getDoctorFirstName(self):
        return self._doctor_first_name

    def setDoctorLastName(self, value):
        self._doctor_last_name = value

    def getDoctorLastName(self):
        return self._doctor_last_name

    def setAcqDate(self, value):
        self._acqDate = value

    def getAcqDate(self):
        return self._acqDate

    def getAcqDateInt(self):
        if self._acqDateInt is None:
            # print("Need to get Acq data int from %s" % self._acqDate)
            parts = self.getAcqDate().split(' ')
            #print("DATE PARTS: %s" % repr(parts))
            d = parts[0]
            # print("DATE: %s" % d)
            parts = d.split('/')
            # print(parts)
            month = int(parts[0])
            day = int(parts[1])
            year = int(parts[2])
            self._acqDateInt = year * 10000 + month * 100 + day
        return self._acqDateInt

    def getAcqTime(self):
        if self._acqTime is None:
            parts = self.getAcqDate().split(' ')

            temp = []
            for part in parts:
                x = part.strip()
                if not x: continue
                temp.append(x)

            # print("TIME PARTS: %s" % repr(temp))

            t = temp[1]

            parts = t.split(":")
            hour = int(parts[0])
            minute = int(parts[1])
            t = "%02d:%02d" % (hour, minute)
            self._acqTime = t
            # print("%s --> %s" % (self.getAcqDate(), self._acqTime))
        return self._acqTime

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

        # print("CALLED WITH %s" % repr(value))
        if value == 'UNKNOWN':
            return

        if value.startswith('ON'):
            self._phnProv = 'ON'
            phn = value[2:]

        elif value.startswith('NS'):
            self._phnProv = 'NS'
            phn = value[2:]

        elif value.startswith('AB'):
            self._phnProv = 'AB'
            phn = value[2:]

        elif value.startswith('BC'):
            self._phnProv = 'BC'
            phn = value[2:]
        else:
            self._phnProv = 'SK'
            phn = value

        # PHN must be an int; otherwise throw an exception
        self._phn = int(phn)

    def getPhn(self):
        return self._phn

    def getPhnProv(self):
        return 'SK'

    def capitalizeName(self, value):

        print("called with %s" % value)
        temp = value.strip()
        temp = temp.lower()

        useDashes = False

        p = temp.split(' ')

        if len(p) > 1:
            parts = p
        else:
            p = temp.split('-')
            if len(p) > 1:
                parts = p
                useDashes = True
            else:
                parts = [temp]

        result = []

        # Capitalize the individual parts
        for part in parts:
            fixed = part.capitalize()
            result.append(fixed)

        result2 = []
        for item in result:
            if item.startswith('Mc'):
                item = 'Mc' + item[2:].capitalize()
            elif item.startswith('Mac'):
                item = 'Mac' + item[3:].capitalize()
            result2.append(item)

        result3 = []
        skip = False
        for i, item in enumerate(result2):
            if skip:
                skip = False
                continue

            try:
                if item == 'O':
                    item = "O'" + result2[i+1]
                    skip = True
            except:
                item = 'O'
                
            result3.append(item)

        if useDashes:
            result = '-'.join(result3)
        else:
            result = ' '.join(result3)

        if result.startswith("O'"):
            result = "O'" + result[2:].capitalize()
        # print("NAME %s ----> %s" % (value, result))
        return result


    def setFirstName(self, value, skip_validate=False):
        if skip_validate:
            self._firstName = value
            return
        self._firstName = self.capitalizeName(value)

    def getFirstName(self):
        return self._firstName

    def setLastName(self, value, skip_validate=False):
        if skip_validate:
            self._lastName = value
            return

        self._lastName = self.capitalizeName(value)

    def getLastName(self):
        return self._lastName

    def setGender(self, value):
        self._gender = value

    def getGender(self):
        return self._gender

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
