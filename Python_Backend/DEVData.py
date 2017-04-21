from collections import OrderedDict


class DEVData:
    def __init__(self, json=None):
        if json is None:
            self.sessionName = self.timeStamp = self.batteryStatus = ''
            self.voltage = self.current = self.speed = self.mileage = \
                self.longitude = self.latitude = self.altitude = self.power = \
                self.energyUsed = self.temperature = self.batteryVoltage = 0.0
            self.msSinceStart = self.heading = 0
            return
        fields = ['sessionName', 'timeStamp', 'voltage', 'current', 'power', 'speed', 'mileage',
                  'longitude', 'latitude', 'altitude', 'msSinceStart', 'heading', 'energyUsed',
                  'batteryStatus', 'temperature', 'batteryVoltage']
        for i in fields:
            if not (i in json):
                raise ValueError
        self.__dict__ = json

    def __str__(self):
        ordered_dict = OrderedDict(self.__dict__.items())
        result = " "
        return result.join('{0}'.format(w) for w in ordered_dict.values())
