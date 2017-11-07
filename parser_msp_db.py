# Created by  Lisa Martini on 10/18/2017


import json
import os
import platform
import subprocess
import sys
import threading
import re
from datetime import *

# !/usr/bin/python
from signal import signal, SIGINT

from pymongo import MongoClient, errors

global parser_process


def interrupt():
    parser_process.stop()
    sys.exit(0)


class ParserProcess(threading.Thread):
    def __init__(self ):
        threading.Thread.__init__(self)
        
        try:
                self.client = MongoClient()
        	self.db = self.client.Hoomie #one database for each residency
        	self.room = self.db.temperatures # for now on only one collection called temperatures
        	print(self.db)
        	print(self.room)
        except (errors.ConnectionFailure, errors.OperationFailure) as e:
        	print "error occurs",e
        	self.stop()
        	sys.exit(0)
        	
        self.__stop__event = threading.Event()

    def stop(self):
        self.__stop__event.set()
        self.client.close()

    def stopped(self):
        return self.__stop__event.is_set()

    def execute(self, cmd):
        popen = subprocess.Popen(['sudo',cmd], stdout=subprocess.PIPE, universal_newlines=True)
        for stdout_line in iter(popen.stdout.readline, ""):
            yield stdout_line
        popen.stdout.close()
        return_code = popen.wait()
        if return_code:
            raise subprocess.CalledProcessError(return_code, cmd)

    def run(self):
        dir_path = os.path.dirname(os.path.realpath(__file__))
        if platform.architecture() == ('32bit', 'ELF'):
            bin_path = os.path.join(dir_path, "ezconsole", "ezconsole_bin_raspberry")
        else:
            bin_path = os.path.join(dir_path, "ezconsole", "ezconsole")
	try :
	
		for line in self.execute(bin_path):  
		              
		    if "node id" in line : 
		    	pass
		    elif "id" and "temperature" and "time" in line :
		    	info = line.replace(',',':').replace('\n','').split(':')
		    	if info[0] == "id":
		    		id = info[1]
		    		currentdate = datetime.today()
		    	if "time" in info :
		    		ms = int(info[info.index("time")+1])
		    		delta = timedelta(milliseconds=ms) 
		    		date = currentdate + delta
		    	if "temperature" in info :
		    		temp = info[info.index("temperature")+1]
		    	if not id:
		    		id = 10
		    	doc ={"sensorId": id, "date": str(date), "temperature": temp}
		    	doc_id = self.room.insert_one(doc).inserted_id 
		    	print "write in db succesful"
	except (RuntimeError, TypeError, NameError, ValueError, IOError, errors.WriteError,errors.WriteConcernError) as e:
		print "error occurs ", e
		self.stop()
		sys.exit(0)
				    
	
   
signal(SIGINT, interrupt)
parser_process = ParserProcess()
parser_process.start()
parser_process.join()
