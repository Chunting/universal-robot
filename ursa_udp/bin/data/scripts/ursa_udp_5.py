#		 USER 				  			  PYTHON				     UR COMPUTER
#						   udp_from port
#	robot commands 		-------- > --------
# 	(e.g. movel(pose))						\		tcp port
#										  	  ------ < -- > ------ robot controller
#	robot data 			    udp_to port 	/
#	(e.g. tcp position) -------- < --------
#

import socket
from struct import pack, unpack
from datetime import datetime as dt
import math

# debug macro
db = True

# TODO: Ask for 2048 bytes from controller 

# *******************************************
# **************** SERVERS ******************
# *******************************************

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
			if (self.lastConnectionAttemptTime is not None):
				diff = dt.now() - self.lastConnectionAttemptTime
				if (diff > self.connectionTimeout):
					print self.type, " reconnecting... via connection timeout"
					# attempt to reconnect
					self.close()
					self.connect()
					self.resetConnectionTimes()
		else:
			# we're connected, so check data activity
			if (self.lastDataExchangeTime is not None):
				diff = dt.now() - self.lastDataExchangeTime
				if (diff > self.dataTimeout):
					print "Reconnecting... via data timeout"
					# we haven't received data in long enough time, so reconnect
					self.close()
					self.connect()
					self.resetConnectionTimes()

# tcp client: receive and send messages
class tcpClient(urSocket):
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		urSocket.__init__(self, host, port, connectionTimeout, dataTimeout)
		# socket.SOCK_STREAM is tcp communication
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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

# udp receive messages
class udpClient(urSocket):
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		urSocket.__init__(self, host, port, connectionTimeout, dataTimeout)
		# socket.SOCK_DGRAM is UDP communication
		urSocket.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.type = "UDP_FROM"

	def setBlocking(self, bBlocking):
		self.sock.setblocking(int(bBlocking))

	# bind to the port to listen for messages
	# Note: udp binding is different than tcp connection
	def connect(self):
		if not self.bConnected:
			self.lastConnectionAttemptTime = dt.now()
			self.lastDataExchangeTime = dt.now() 
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

	# returns message if there is any
	def getMessage(self, messageSize):
		try: 
			data, addr = self.sock.recvfrom(messageSize)
			print self.type, " received message \"", data, "\""
			if (len(data) > 0):
				self.lastDataExchangeTime = dt.now()
			return data
		except:
			return None

# udp send messages
class udpServer(urSocket):
	def __init__(self, host, port, connectionTimeout, dataTimeout):
		urSocket.__init__(self, host, port, connectionTimeout, dataTimeout)
		urSocket.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
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
		self.tcpActualPosition = [doubleData[55], doubleData[56], doubleData[57]]
		self.tcpTargetPosition = [doubleData[73], doubleData[74], doubleData[75]]

		dx = self.tcpActualPosition[0] - self.tcp

def bytesToDoubles(byteData, nDoubles):
	doubles = []
	for i in xrange(nDoubles):
		sum = 0
		for j in xrange(8):
			index = 4 + i * 8 + (7 - j)
			sum += byteData[index]*256**j
		thisDouble = unpack("d", pack("L", sum))[0]
		doubles.append(thisDouble)
	return doubles

# handles all connection across three clients / servers
class urServer:
	def __init__(self, tcp_host, tcp_port, udp_host, udp_from_port, udp_to_port, connectionTimeout, dataTimeout):
		self.tcp = tcpClient(tcp_host, tcp_port, connectionTimeout, dataTimeout)
		self.udpFrom = udpClient(udp_host, udp_from_port, connectionTimeout, dataTimeout)
		self.udpTo = udpServer(udp_host, udp_to_port, connectionTimeout, dataTimeout)

		# leftover data from previous (unfinished) messages from robot
		self.lastData = []

		# debug number of packets sent
		self.packetSentLastSecond = 0
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

	def getRobotData(self, expectedMessageSize):
		# don't attempt to get data if we're not connected
		if (not self.tcp.bConnected): return None

		# first get data
		rawData = self.tcp.getMessage(2048)
		if (rawData == None): return None

		# mark that this socket is active
		self.tcp.lastDataExchangeTime = dt.now()

		# parse data
		rawData = ":".join("{:02d}".format(ord(c)) for c in rawData)
		byteData = [int(x.strip()) for x in newData.split(':')]
		nBytes = len(byteData)

		if (nBytes > 0):
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









		if (nBytes >= bytesInMessage):
			# We have sufficient data to be able to send a message
			if (len(self.lastData) > 0):
				# Use the remaining data


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




		nDoubles = 132
		doubles = []


		return data

	# get user commands from udpFrom and perform the appropriate
	# system operations
	def exchangeUserCommands(self):

		return commands

	# def sendToRobot(self, message):




myServer = urServer("192.168.1.9", 30003, "127.0.0.1", 5001, 5002, 5000, 5000)
myServer.connectAll()
myServer.setBlockingAll(False)

while (True):
	myServer.updateAllConnections()

	myServer.getUserCommands()
	data = myServer.getRobotData(1060)

	myServer.sendUserCommands() # when available
	myServer.sendRobotData(data) # when available




