from my_logger import MyLogger

log = MyLogger(debug_level=1)

class Doctors(object):
    def __init__(self, sql):
        log.dbg(1, "doctors")
        self._sql = sql

        self._doctors = []

    def load(self):

        cmd = "select id, first_name, last_name, doctor_number from doctors " \
                "where not_deleted=1 and on_msp_list=1 and doctor_number > 0"

        rows = self._sql.select(cmd)
        for row in rows:
            # print(row)
            doctor_id = int(row[0])
            first_name = row[1]
            last_name = row[2]
            doctor_number = int(row[3])
            self._doctors.append((last_name.lower(), last_name, first_name, doctor_id, doctor_number))

        self._doctors.sort()

        # for item in self._doctors:
        #     print(item)

    def search(self, s):

        if not s: return []
        s = s.strip().lower()
        if not s: return []

        result = []

        for doctor in self._doctors:
            last_name_lower = doctor[0]

            if last_name_lower.find(s) >= 0:
                result.append(doctor)

        # for doctor in result:
        #     print("GOT: %s" % repr(doctor))

        return result