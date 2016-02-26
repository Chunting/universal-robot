# Companion Script for OpenFrameworks App "ursa_udp"
# Ben Snell - 19/02/2016

# v3 rewritten to issue a new string command right before the 
# robot has reached the position described in the previous command

# Designed to work with UR Port 30003 (real-time client)

# Relays string commands to UR5 over udp on port 5002
# Relays robot data from UR5 over udp on port 5001 
# (doubles formatted as a string)

# Note: Requires direct wired connection over ethernet (wifi is too slow)

# Note: after issuing a move command, it takes 30-40 ms to start moving
# again (if changing directions completely)

# TODO: Only issue a command if a a command has not been previously issued
# TODO: When a new command is issued, parse out its start / end point
# TODO: Attempt to reconnect to tcp / udp every 3 seconds
# TODO: Cycle through udp ports if not connecting
# TODO: start app with given udp from OF if specified

import socket
from struct import pack, unpack
from datetime import datetime
import math

# TCP to robot
TCP_HOST = "192.168.1.9"
TCP_PORT = 30003

# UDP
UDP_HOST = "127.0.0.1"
# to OF
UDP_PORT_TO = 5005
# from OF
UDP_PORT_FROM = 5004

# initialize sockets
tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
udpTo = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udpFrom = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# set timeout so the program doesn't hang on receiving commands

udpFrom.setblocking(0) # equivalent to a timeout of 0.0
# should it hang on the robot commands? For how long?

# connect to the robot (tcp) and open up the udp from port
try: 
	tcp.connect((TCP_HOST, TCP_PORT))
	print "TCP OPEN"
except:
	try:
		udpFrom.bind((UDP_HOST, UDP_PORT_FROM))
		print "UDP_TO OPEN"
		tcp.connect((TCP_HOST, TCP_PORT))
		print "TCP OPEN"
	except:
		print "COULD NOT CONNECT TO TCP. CLOSING PROGRAM"
		tcp.close()
		udpFrom.close()
		udpTo.close()
		exit()
try:
	udpFrom.bind((UDP_HOST, UDP_PORT_FROM))
	print "UDP_TO OPEN"
except:
	print "COULD NOT CONNECT TO UDP. CLOSING PROGRAM"
	tcp.close()
	udpFrom.close()
	udpTo.close()
	exit()

tcp.setblocking(0)

# store unfinished messages
lastData = []

packetsSent = 0
thisSecond = 0

# first in first out buffer of commands to robot
cmdQueue = []
# bool to know if robot is in motion
bRobotInMotion = False

# bool know whether a command has recently been issued
lastCmdIssuedTime = datetime.now()
# command issued duration threshold (microseconds) (usually takes 30 ms to issue a command)
waitTime = 40000

# cmdIssued = False

# misc time params
# t1 = datetime.now()
# t2 = datetime.now()
# t3 = datetime.now()

# current tcp pose
tcpPose = []
# target tcp pose
targetTcpPose = []
# distance threshold by which a pose is considered to have been completed
distThresh = 0.003 # 3 cm
# motion almost finished
bMotionAlmostFinished = False

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

class urCommand:
	def __init__(self, strCmd, startPose = None):
		self.cmd = strCmd
		self.start = []
		if (startPose is not None):
			self.start = startPose	

# parse command depending on the movement command given
def addCommandToQueue(command, queue):
	# add string command
	if ("\n" not in command): command += "\n"

	# add point at which movement begins
	startPose = []
	if (command.find("move") >= 0):
		# move command, so has a start point
		startIndex = command.find("[") + 1
		endIndex = command.find("]")
		parsedCmd = (command[startIndex:endIndex]).split(",")
		for num in parsedCmd:
			startPose.append(float(num))

	myCmd = urCommand(command, startPose)
	queue.append(myCmd)

	print "Added new command (", command[:len(command)-2], "). There are now ", len(queue), " commands in queue."

	# if there's nothing in the queue and the robot is not in motion, then 
	# update the target tcp

# call to update knowledge of where tcp is and whether robot is moving
def updateRobotPosition(doubles, bRobotInMotion, tcpPose, targetTcpPose, bMotionAlmostFinished, distThresh):

	# if we have new data...
	if (len(doubles) > 0):	

		# ************** IN MOTION ******************

		# update whether robot is is motion
		precision = 100 # precision of rounding (100 seems good)
		bPrevRobotInMotion = bRobotInMotion
		bRobotInMotion = not (int(doubles[118]*precision) == 0)
		# look at transition from not moving to moving


		# ********* MOTION ALMOST FINISHED **********

		# update where the robot tcp is
		tcpPose = doubles[55:61]
		# find the distance from the tcp to the target position
		dx = cmdQueue[0].start[0] - tcpPose[0]
		dy = cmdQueue[0].start[1] - tcpPose[1]
		dz = cmdQueue[0].start[2] - tcpPose[2]
		dist = math.sqrt(dx*dx + dy*dy + dz*dz)
		# update whether this robot is within the threshold
		if (dist < distThresh):
			bMotionAlmostFinished = True

		# if the last command was issued 

		# if the robot has started moving, then 
		# if (not bPrevRobotInMotion and bRobotInMotion):
		# 	cmdIssued = False # accounts for delay in movement after issuing a command

def issueCommand(tcp, cmdQueue, tcpPose):

	# if we don't have knowledge of where the robot is, or if we don't
	# have commands to issue, don't issue the command
	if (len(tcpPose) == 0 or len(cmdQueue) == 0): 
		return

	# Empty the queue if:
		# the queue still has commands in it, OR
		# the robot is within a specied distance (threshold) of the target position




		# if the robot isn't moving, then issue the command
		if (not bRobotInMotion and not cmdIssued):
			print "Command issued (", cmdQueue[0].cmd[:len(cmdQueue[0].cmd)-2], "), because robot is not in motion. Dist is ", dist, " m."
			tcp.send(cmdQueue[0].cmd)
			cmdIssued = True
			cmdQueue.pop(0)
			print "There are ", len(cmdQueue), " commands remaining to execute."
		elif (dist <= distThresh):
			print "Command issued (", cmdQueue[0].cmd[:len(cmdQueue[0].cmd)-2], "), because robot is within ", distThresh, " m at ", dist, " m."
			cmdIssued = False
			tcp.send(cmdQueue[0].cmd)
			cmdIssued = True
			cmdQueue.pop(0)
			print "There are ", len(cmdQueue), " commands remaining to execute."

	# record the time at which this command is issued

	# save the target tcp pose for the next command




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
			# this is a command to the robot
			# parse the command list into separate commands, separated by 
			# end of line characters
			while (len(cmd) > 0):
				eolIndex = cmd.find("\n")
				if (eolIndex == -1):
					# assume it is only one command
					addCommandToQueue(cmd, cmdQueue)
					cmd = []
					print "added a command with no line ending"
				else:
					# eol found, so take string up to it
					addCommandToQueue(cmd[:(eolIndex+2)], cmdQueue)
					# delete those characters
					cmd = cmd[(eolIndex+2):]
					print "added a command in a multiline script"

	# issue commands
	issueCommand(tcp, cmdQueue)

	# # Empty the queue if:
	# 	# the queue still has commands in it
	# 	# the robot is within a specied distance (threshold) of the target position
	# if (len(cmdQueue) > 0 and len(tcpPose) > 0):
	# 	# find distance
	# 	dx = cmdQueue[0].start[0] - tcpPose[0]
	# 	dy = cmdQueue[0].start[1] - tcpPose[1]
	# 	dz = cmdQueue[0].start[2] - tcpPose[2]
	# 	dist = math.sqrt(dx*dx + dy*dy + dz*dz)

	# 	# if the robot isn't moving, then issue the command
	# 	if (not robotInMotion and not cmdIssued):
	# 		print "Command issued (", cmdQueue[0].cmd[:len(cmdQueue[0].cmd)-2], "), because robot is not in motion. Dist is ", dist, " m."
	# 		tcp.send(cmdQueue[0].cmd)
	# 		cmdIssued = True
	# 		cmdQueue.pop(0)
	# 		print "There are ", len(cmdQueue), " commands remaining to execute."
	# 	elif (dist <= distThresh):
	# 		print "Command issued (", cmdQueue[0].cmd[:len(cmdQueue[0].cmd)-2], "), because robot is within ", distThresh, " m at ", dist, " m."
	# 		cmdIssued = False
	# 		tcp.send(cmdQueue[0].cmd)
	# 		cmdIssued = True
	# 		cmdQueue.pop(0)
	# 		print "There are ", len(cmdQueue), " commands remaining to execute."


	# # Empty queue if robot is not in motion
	# # NOTE: This will not work for continuous motion operations!!!
	# # Robot may not be in motion even if a command has been issued, so:
	# 	# wait for robot to move (or wait for a timeout to end)
	# # timeout to end to issue another command
	# if (len(cmdQueue) > 0 and not robotInMotion and not cmdIssued):
	# 	# send first command in queue
	# 	tcp.send(cmdQueue[0])
	# 	cmdIssued = True
	# 	# delete first command
	# 	cmdQueue.pop(0)
	# 	print "there are ", len(cmdQueue), " commands remaining to execute"
	# 	# save this time to see how long it takes to issue and respond to a command
	# 	# t2 = datetime.now()

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
	# **** UPDATE ROBOT POSITION ****
	# *******************************

	updateRobotPosition(doubles, bRobotInMotion, tcpPose)

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


		# if robot has stopped moving, time how long it takes to issue the command
		# if (not robotInMotion and prevRobotInMotion):
		# 	t1 = datetime.now()
		# if (not prevRobotInMotion and robotInMotion):
		# 	t3 = datetime.now()
		# 	# print info about timing
		# 	tCmdDelay = t2 - t1
		# 	tMoveDelay = t3 - t2
		# 	print "command delay: ", tCmdDelay.microseconds, "    move delay: ", tMoveDelay.microseconds

	# *******************************
	# *********** DEBUG *************
	# *******************************

	# print info about packets sent
	timeNow = datetime.now()
	if (timeNow.second is not thisSecond):
		print packetsSent, " packets sent at", datetime.now().time()
		packetsSent = 0
		thisSecond = timeNow.second


