'''
Open Source Initiative OSI - The MIT License:Licensing
Tue, 2006-10-31 04:56 nelson

The MIT License

Copyright (c) 2009 BK Precision

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

This script talks to the DC load in two ways:
    1.  Using a DCLoad object (you'd use this method when you write a
        python application that talks to the DC load.

    2.  Using the COM interface.  This shows how python code uses the
        COM interface.  Other programming environments (e.g., Visual
        Basic and Visual C++) would use very similar techniques to
        talk to the DC load via COM.

Note that the DCLoad object and the COM server interface functions
always return strings.

$RCSfile: client.py $ 
$Revision: 1.0 $
$Date: 2008/05/16 21:02:50 $
$Author: Don Peterson $
'''

import sys, dcload, csv
import time
try:
    from win32com.client import Dispatch
except:
    pass
err = sys.stderr.write

def TalkToLoad(load, port, baudrate, lapData):
    '''load is either a COM object or a DCLoad object.  They have the 
    same interface, so this code works with either.
 
    port is the COM port on your PC that is connected to the DC load.
    baudrate is a supported baud rate of the DC load.
    '''
    def test(cmd, results):
        if results:
            print cmd, "failed:"
            print "  ", results
            exit(1)
        else:
            print cmd

    loadOnPrev = True

    load.Initialize(port, baudrate) # Open a serial connection
    print "Time from DC Load =", load.TimeNow()
    test("Set to remote control", load.SetRemoteControl())

    test("Set max current to 7 A", load.SetMaxCurrent(7))
    test("Set to CW", load.SetMode("cw"))
    test("Set CW to 73W", load.SetCWPower(73))

    print "Test starting"
    startTime = getSeconds(0)

    endTime = 0
    for interval in lapData:
        endTime = max(endTime, interval[1])

    print "End time is ", endTime

    while getSeconds(startTime) < endTime + 50:
        curTime = getSeconds(startTime)

        loadOn = False

        for interval in lapData:
            if(curTime > interval[0] and curTime < interval[1]):
                loadOn = True;

        if loadOn and not loadOnPrev: #the command has a 0.1% chance of crashing, so reducing the number of times its called
            load.TurnLoadOn()
        if not loadOn and loadOnPrev:
            load.TurnLoadOff()

        loadOnPrev = loadOn

        printf("Time %6.2f, dcLoadOn=%s ", curTime, loadOn)

        time.sleep(0.3)
        print " . "


def getSeconds(startTime):
    return time.time() - startTime

def main():
    lapData = readCSV()

    port        = "23"
    baudrate    = "38400"

    load = dcload.DCLoad()

    TalkToLoad(load, port, baudrate, lapData)
    return 0

def readCSV():
    lapData = []

    file = "lapData.txt"
    with open(file, "rb") as csvfile:
        reader = csv.reader(csvfile)

        for row in reader:
            rowData = map(float, row)
            lapData.append(rowData)

    return lapData

def printf(format, *args):
    sys.stdout.write(format % args)

main()
