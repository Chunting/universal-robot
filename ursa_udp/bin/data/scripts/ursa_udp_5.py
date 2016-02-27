#		 USER 				  			  PYTHON				     UR COMPUTER
#						   udp_from port
#	robot commands 		-------- > --------
# 	(e.g. movel(pose))						\		tcp port
#										  	  ------ < -- > ------ robot controller
#	robot data 			    udp_to port 	/
#	(e.g. tcp position) -------- < --------
#

# Optional:
# run script with two arguments to specify udp_from_port and udp_to_port:
# python ursa_udp_5.py 5005 5006

import socket
from struct import pack, unpack
from datetime import datetime as dt
import math
from sys import argv

# *******************
# ***** SERVERS *****
# *******************

# primary class to handle connection / data timeout
class urSocket:
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		self.sock = None
		self.host = host
		self.port = port
		self.addr = (self.host, self.port)
		self.type = None

		# time it takes to try to reestablish connection (ms)
		self.connectionTimeout = connectionTimeout
		# last time a connection was attempted
		self.lastConnectionAttemptTime = None
		# bool to know whether socket is officially open or closed
		self.bConnected = False
		# direct flag to attempt reconnection
		self.flagReconnect = False

		# time it takes of no data exchange to close socket 
		# and attempt reconnection (ms)
		self.dataTimeout = dataTimeout
		# last time data was received or successfully sent
		self.lastDataExchangeTime = None
	
	# set timeout for server (amount of time to wait for a message)
	def setBlocking(self, bBlocking):
		# 0 for timeout of 0 ms (does not hang)
		self.sock.setblocking(int(bBlocking))

	# close connection (same across all types of sockets)
	def close(self):
		if self.bConnected:
			print self.type, " closing on port ", self.port
			self.sock.close()
			self.bConnected = False
		else:
			print self.type, " socket is already closed, but closing again"
			self.sock.close()
			self.bConnected = False

	def resetConnectionTimes(self):
		self.lastConnectionAttemptTime = dt.now()
		self.lastDataExchangeTime = dt.now()

	# attempt to reconnect when necessary
	def updateConnection(self):
		# if flag reconnect has been set, attempt reconnection
		if (self.flagReconnect):
			print "Reconnecting..."
			self.close()
			self.connect()
			self.resetConnectionTimes()
			return None

		# otherwise, first check connection activity
		if not self.bConnected:
			if (self.lastConnectionAttemptTime is not None and self.connectionTimeout != -1):
				diff = dt.now() - self.lastConnectionAttemptTime
				if (diff.total_seconds() * 1000 > self.connectionTimeout):
					print self.type, " reconnecting... via connection timeout"
					# attempt to reconnect
					self.close()
					self.connect()
					self.resetConnectionTimes()
		else:
			# we're connected, so check data activity
			if (self.lastDataExchangeTime is not None and self.dataTimeout != -1):
				diff = dt.now() - self.lastDataExchangeTime
				if (diff.total_seconds() * 1000 > self.dataTimeout):
					print self.type, " reconnecting... via data timeout"
					# we haven't received data in long enough time, so reconnect
					self.close()
					self.connect()
					self.resetConnectionTimes()

# tcp client: receive and send messages
class tcpClient(urSocket):
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		urSocket.__init__(self, host, port, connectionTimeout, dataTimeout)
		# socket.SOCK_STREAM is tcp communication
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # not urSocket.sock
		self.type = "TCP"

	def setBlocking(self, bBlocking):
		# 0 for timeout of 0 ms (does not hang)
		self.sock.setblocking(int(bBlocking))

	# attempt to connect; returns success of attempt
	# Note: tcp connection is different than udp binding
	def connect(self):
		if not self.bConnected:
			# so we don't also disconnect because of lack of data if we are 
			# already having connection issues:
			self.resetConnectionTimes()
			try: 
				self.sock.connect(self.addr)
				self.bConnected = True
				print self.type, " connected on port ", self.port
				return True
			except:
				print self.type, " could not connect on port ", self.port
				return False
		else:
			print self.type, " is already connected on port ", self.port
			return False

	# get any waiting messages (attempts to receive messageSize bytes), 
	# return the message
	def getMessage(self, messageSize):
		if self.bConnected:
			try:
				message = self.sock.recv(messageSize)
				print self.type, " received message with ", len(message), " / ", messageSize, " bytes"
				if (len(message) > 0):
					self.lastDataExchangeTime = dt.now()
				return message
			except:
				return None
		else:
			print self.type, " could not get message because socket isn't connected"
			return None

	# send a message; returns number of bytes sent
	def sendMessage(self, message):
		if self.bConnected:
			if (len(message) == 0):
				print self.type, " did not send message because message contains nothing"
				return 0
			bytesSent = self.sock.send(message)
			if (bytesSent > 0):
				# data successfully sent
				self.lastDataExchangeTime = dt.now()
				print self.type, " sends ", bytesSent, " / ", len(message), " bytes with message \"", message, "\""
				return bytesSent
			else:
				# channel is closed (not active)
				self.flagReconnect = True
				print self.type, " sent received 0 bytes, so flag a reconnect"
				return bytesSent
		else:
			print self.type, " not connected, so didn't send message \"", message, "\""
			return 0

# udp client to receive messages
class udpClient(urSocket):
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		urSocket.__init__(self, host, port, connectionTimeout, dataTimeout)
		# socket.SOCK_DGRAM is UDP communication
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		# prevent socket from not connecting if socket is already in use
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.type = "UDP_FROM"

	def setBlocking(self, bBlocking):
		self.sock.setblocking(int(bBlocking)) # Why is this urSocket???

	# bind to the port to listen for messages
	# Note: udp binding is different than tcp connection
	def connect(self):
		if not self.bConnected:
			self.resetConnectionTimes()
			try: 
				self.sock.bind(self.addr)
				self.bConnected = True
				print self.type, " connected on port ", self.port
				return True
			except:
				print self.type, " could not connect on port ", self.port
				return False
		else:
			print self.type, " is already connected on port ", self.port
			return False

	def getMessage(self, messageSize):
		if self.bConnected:
			try: 
				data, addr = self.sock.recvfrom(messageSize)
				print self.type, " received message \"", data, "\""
				if (len(data) > 0):
					self.lastDataExchangeTime = dt.now()
				return data
			except:
				return None
		else:
			# print self.type, " could not get message because socket isn't connected"
			return None

# udp server to send messages
class udpServer(urSocket):
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		urSocket.__init__(self, host, port, connectionTimeout, dataTimeout)
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.type = "UDP_TO"

	# no need to set blocking
	# no need to connect or bind

	# send message, returns number of bytes
	def sendMessage(self, message):
		if self.bConnected:
			if (len(message) == 0):
				print self.type, " did not send message because message contains nothing"
				return 0
			bytesSent = self.sock.sendto(message, self.addr)
			if (bytesSent > 0):
				# data successfully sent
				self.lastDataExchangeTime = dt.now()
				print self.type, " sends ", bytesSent, " / ", len(message), " bytes with message \"", message, "\""
				return bytesSent
			else:
				# channel is closed (not active)
				self.flagReconnect = True
				print self.type, " sent received 0 bytes, so flag a reconnect"
				return bytesSent
		else:
			print self.type, " not connected, so didn't send message \"", message, "\""

# handles command queue and updating movement data
class robot:
	def __init__(self):
		# holds the list of commands waiting to be executed
		self.commandQueue = []

		# self.robotInMotionPrecision = 100 # higher number, greater precision
		# self.bRobotInMotion = False

		# most recent positions of the tcp
		self.tcpActualPosition = []
		self.tcpTargetPosition = [] # end of path

		self.actualToTargetTcpDistance = 0 # None?

		# distance from the actual tcp to the target position it takes to trigger
		# sending the next command to the robot (meters)
		self.commandTriggerDistance = 0.01

		# if false, move is in progress
		# if true, movement is not in progress (either robot isn't moving or is
		# within the trigger distance of target position to trigger the next command)
		self.bMoveInProgress = False

	def updatePosition(self, doubleData):
		self.tcpActualPosition = doubleData[55:57]
		self.tcpTargetPosition = doubleData[73:75]

		dx = self.tcpActualPosition[0] - self.tcpTargetPosition[0]
		dy = self.tcpActualPosition[1] - self.tcpTargetPosition[1]
		dz = self.tcpActualPosition[2] - self.tcpTargetPosition[2]
		self.actualToTargetTcpDistance = math.sqrt(dx*dx + dy*dy + dz*dz)

		if (self.actualToTargetTcpDistance <= self.commandTriggerDistance):
			self.bMoveInProgress = False
		else:
			self.bMoveInProgress = True

	def addCommand(self, command):
		if (command.find("\n") < 0):
			command += "\n"
		self.commandQueue.append(command)

	# returns next command in queue (fifo)
	# keeps rest of commands to return later
	def getNextCommand(self):
		if (len(self.commandQueue) == 0): return None
		command = self.commandQueue[0]
		self.commandQueue.pop(0)
		return command

	# returns the last command added to the queue (lifo + empty queue)
	# empties rest of commands received before this one
	def getLastCommand(self):
		if (len(self.commandQueue) == 0): return None
		command = self.commandQueue[-1]
		self.commandQueue = []
		return command

# turn a series of bytes into doubles (big endian)
def bytesToDoubles(byteData, nDoubles):
	doubleData = []
	for i in xrange(nDoubles):
		sum = 0
		for j in xrange(8):
			index = 4 + i * 8 + (7 - j)
			sum += byteData[index]*256**j
		thisDouble = unpack("d", pack("L", sum))[0]
		doubleData.append(thisDouble)
	return doubleData

# handles all connection across three clients / servers
class urServer:
	def __init__(self, tcp_host, tcp_port, tcp_connectionTimeout, tcp_dataTimeout, udp_host, udp_from_port, udp_to_port, udp_connectionTimeout, udp_dataTimeout):
		self.tcp = tcpClient(tcp_host, tcp_port, tcp_connectionTimeout, tcp_dataTimeout)
		self.udpFrom = udpClient(udp_host, udp_from_port, udp_connectionTimeout, udp_dataTimeout)
		self.udpTo = udpServer(udp_host, udp_to_port, udp_connectionTimeout, udp_dataTimeout)

		# leftover data from previous (unfinished) messages from robot
		self.lastData = []

		# debug number of packets sent
		self.packetsSentLastSecond = 0
		self.thisSecond = 0

		# robot object contains robot-related data
		self.robot = robot()

	def setBlockingAll(self, bBlocking):
		self.tcp.setBlocking(0)
		self.udpFrom.setBlocking(0)
		# no need to set blocking for sending udp

	def connectAll(self):
		self.tcp.connect()
		self.udpFrom.connect()
		# no need to connect to send udp commands

	def closeAll(self):
		self.tcp.close()
		self.udpFrom.close()
		self.udpTo.close()

	def closeProgram(self):
		self.closeAll()
		exit()

	def updateAllConnections(self):
		# check to see if connections are active and attempt to reestablish connection where appropriate
		self.tcp.updateConnection()
		self.udpFrom.updateConnection()

	# checks for program keys in a string and issues them
	# returns false if program key isn't found
	def checkForProgramKey(self, command):
		if (command == "c"):
			self.closeAll()
		elif (command == "q" or command == 'e'):
			self.closeAll()
			exit()
		elif (command == 'o'):
			self.connectAll()
		# can add more program keys here with more elif statements
		else:
			return False
		print "Received message: \"" + command + "\""
		return True

	def getUserCommands(self, bufferSize):
		# get raw data, if there is any
		data = self.udpFrom.getMessage(bufferSize)
		if (data is None): return None

		# parse commands and store them in the queue
		splitData = data.split("\n")
		splitData = filer(None, splitData)
		for command in splitData:
			if (checkForProgramKey(command)):
				# if a key wasn't found, add the command to the queue
				self.robot.addCommand(command)
				print "Received message: \"" + command + "\""

	def sendUserCommands(self, bEmptyQueue, bInterruptMovements):
		if (not bInterruptMovements and self.robot.bRobotInMotion): return False
		command = self.robot.getNextCommand()
		while (command is not None):
			self.tcp.sendMessage(command)
			if bEmptyQueue: 
				command = self.robot.getNextCommand()
			else:
				command = None
		return True

	def getRobotData(self, bufferSize, expectedMessageSize):
		# don't attempt to get data if we're not connected
		if (not self.tcp.bConnected): return None

		# first get data
		rawData = self.tcp.getMessage(bufferSize)
		if (rawData == None): return None

		# mark that this socket is active
		self.tcp.lastDataExchangeTime = dt.now()

		# parse data
		rawData = ":".join("{:02d}".format(ord(c)) for c in rawData)
		byteData = [int(x.strip()) for x in newData.split(':')]
		nBytes = len(byteData)

		# if no data, return none
		if (nBytes == 0): return None

		# we have data; append it to last data
		self.lastData.extend(byteData[:])

		doubleData = []

		# if the length of last data is at least the size of the expected
		# message size, then turn it into doubles
		while (len(self.lastData) >= expectedMessageSize):
			
			# first complete packet of bytes
			firstPacket = self.lastData[:expectedMessageSize]

			# create the doubles
			doubleData = bytesToDoubles(firstPacket, 132)

			# delete this packet from the running list of bytes received
			self.lastData = self.lastData[expectedMessageSize:]

		if (len(doubleData) > 0): 
			# at least one full set of robot data has been received
			self.robot.updatePosition(doubleData)

			return doubleData

	def robotDataPacketSent(self):
		self.packetsSentLastSecond += 1

		timeNow = dt.now()
		if (timeNow.second is not self.thisSecond):
			print packetsSent, "packets sent at", datetime.now().time()
			packetsSent = 0
			self.thisSecond = timeNow.second

	def sendRobotData(self, doubleData):
		if doubleData is None: return None

		# send the data to the user over udp as a comma separated string
		message = str(doubleData[0])
		for double in doubleData:
			message += "," + str(double)
		self.udpTo.sendMessage(message)

		# keep track of packets sent per second (should be 125 Hz)
		self.robotDataPacketSent()

def printStartUpMessage():
	print "UDP Commands: \'q\' to quit, \'c\' to close sockets, \'o\' to open sockets"

def getSystemArguments():
	udp_from_port = 5001
	udp_to_port = 5002
	if (len(argv) == 3):
		udp_from_port = argv[1]
		udp_to_port = argv[2]
	return udp_from_port, udp_to_port

# ************************
# ***** MAIN PROGRAM *****
# ************************

printStartUpMessage()

_udp_from_port, _udp_to_port = getSystemArguments()

myServer = urServer("192.168.1.9", 30003, 5000, 3000, "127.0.0.1", _udp_from_port, _udp_to_port, -1, -1)
myServer.setBlockingAll(False)
myServer.connectAll()

while (True):

	# update socket states to make sure they are connected and active
	myServer.updateAllConnections()

	# get user commands (execute program keys and add robot commands to queue)
	myServer.getUserCommands(1024)

	# get robot data
	data = myServer.getRobotData(2048, 1060)

	# send user commands to robot
	myServer.sendUserCommands(False, True)

	# if we have data, send it to the user
	myServer.sendRobotData(data)

