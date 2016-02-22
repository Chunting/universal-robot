# Companion Script for OpenFrameworks App "ursa_udp"
# Ben Snell - 19/02/2016

# Designed to work with UR Port 30003 (real-time client)

# Relays string commands to UR5 over udp on port 5002
# Relays robot data from UR5 over udp on port 5001 
# (doubles formatted as a string)

# Note: Requires direct wired connection over ethernet (wifi is too slow)

import socket
from struct import pack, unpack
from datetime import datetime

# TCP to robot
TCP_HOST = "192.168.1.9"
TCP_PORT = 30003

# UDP
UDP_HOST = "127.0.0.1"
# to OF
UDP_PORT_TO = 5001
# from OF
UDP_PORT_FROM = 5002

# initialize sockets
tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
udpTo = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udpFrom = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# set timeout so the program doesn't hang on receiving commands
udpFrom.setblocking(0) # equivalent to a timeout of 0.0
# should it hang on the robot commands? For how long?

# connect to the robot (tcp) and open up the udp from port
tcp.connect((TCP_HOST, TCP_PORT))
udpFrom.bind((UDP_HOST, UDP_PORT_FROM))


print "Connected to udp and tcp successfully"

tcp.setblocking(0)

# store unfinished messages
lastData = []

packetsSent = 0
thisSecond = 0

def bytesToDoubles(ints, nDoubles):
	doubles = []
	for i in xrange(nDoubles):
		sum = 0
		for j in xrange(8):
			index = 4 + i * 8 + (7 - j)
			sum += ints[index]*256**j
		thisDouble = unpack("d", pack("L", sum))[0]
		doubles.append(thisDouble)
	return doubles


while(True):

	# *******************************
	# ***** COMMANDS FROM UDP *******
	# *******************************

	# check if a command has been sent over udp
	data = None
	addr = None
	try:
		data, addr = udpFrom.recvfrom(1024)
	except:
		None
		# print "no data received from udp"
	# interpret the command
	if (data != None and len(data) > 0):
		# data received
		cmd = str(data)
		print "Cmd received: " + cmd

		# check if string is a special keyword
		if (cmd == "close"):
			tcp.close()
		elif (cmd == "exit"):
			tcp.close()
			udpFrom.close()
			udpTo.close()
			exit()
		elif (cmd == "open"):
			try:
				tcp.connect((TCP_HOST, TCP_PORT)) # how do you reconnect?
			except:
				print "Could not reconnect to tcp"
		else:
			# reformat command if necessary
			if ("\n" not in cmd): cmd += "\n"
			# send command to robot over tcp port 30003
			tcp.send(cmd)

	# *******************************
	# ****** DATA FROM ROBOT ********
	# *******************************

	# get data from the robot
	# data comes in big-endian
	msgSize = 1060; # bytes in a message
	nDoubles = 132
	doubles = []

	# tstart = datetime.now()
	rawData = None
	try:
		rawData = tcp.recv(msgSize)
	except:
		# print "tcp did not respond in time"
		None
	# tend = datetime.now()
	# tdiff = tend - tstart
	# if (tdiff.microseconds > 10000):
	# 	print "time: \t" + str(tdiff.microseconds)

	if (rawData != None):

		newData = ":".join("{:02d}".format(ord(c)) for c in rawData)
		ints = [int(x.strip()) for x in newData.split(':')]
		nInts = len(ints)
		nBytes = ints[0]*256**3 + ints[1]*256**2 + ints[2]*256**1 + ints[3]

		# print "there are " + str(nBytes) + " bytes and " + str(len(ints)) + " ints"

		if (nBytes == msgSize):
			
			# Beginning of message is found
			# The data in this message is usable

			if (nInts == msgSize):

				# All data has arrived. Export doubles
				doubles = bytesToDoubles(ints, nDoubles)
				# print doubles[0], "\t beginning of message and all data arrived"

			else:

				# Not all data has arrived. Save it in last data for later
				lastData = ints

		else:

			# Beginning of message is not found

			if (len(lastData) > 0):

				# There is remaining data from the last message(s)
				lengthNewData = nInts
				amtRemainingDataNeeded = 1060 - len(lastData)

				if (amtRemainingDataNeeded < lengthNewData):

					# There is more than sufficient new data to complete remaining data
					# from previous messages

					lastData.extend(ints[0:amtRemainingDataNeeded])
					doubles = bytesToDoubles(lastData, nDoubles)
					# print doubles[0], "\t complete remaining data with leftovers"
					lastData = ints[amtRemainingDataNeeded:]

				elif (amtRemainingDataNeeded == lengthNewData):

					# There is exactly sufficient new data to complete remaining data
					# from previous messages

					lastData.extend(ints)
					doubles = bytesToDoubles(lastData, nDoubles)
					# print doubles[0], "\t complete remaining data perfectly"
					lastData = []

				else:

					# There is insufficient data to complete remaining data
					# Add the new data to lastData

					lastData.extend(ints)
					# print "data extended for 2+ times"

			else:

				None
				# print "caught in no-man's land. continue retrieving data until beginning of msg found"

	# *******************************
	# ***** SEND DATA OVER UDP ******
	# *******************************

	if (len(doubles) > 0):
		# turn doubles into a string
		doubleStr = str(doubles[0])
		for i in xrange(1, nDoubles):
			doubleStr += "," + str(doubles[i])
		udpTo.sendto(doubleStr, (UDP_HOST, UDP_PORT_TO))

		# update number of packets sent
		packetsSent += 1


	# print info about packets sent
	timeNow = datetime.now()
	if (timeNow.second is not thisSecond):
		print packetsSent, " packets sent at", datetime.now().time()
		packetsSent = 0
		thisSecond = timeNow.second


