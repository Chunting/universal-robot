import socket
from struct import pack, unpack
from datetime import datetime as dt
import math

# debug macro
db = True

# *******************************************
# **************** SERVERS ******************
# *******************************************

class tcpClient:
	def __init__(self, host, port, commectionTimeout, dataTimeout):
		self.host = host
		self.port = port
		self.addr = (host, port)
		# socket.SOCK_STREAM is TCP communication
		self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		# ms it takes to try to reestablish connection
		self.connectionTimeout = connectionTimeout
		# last time a connection was attempted
		self.lastConnectionAttemptTime = None
		# bool to know whether socket has been connected or closed by us
		self.bConnected = False # public
		# direct flag to attempt reconnection
		self.flagReconnect = False

		# ms it takes of no data exhange to close socket and attempt reconnection
		self.dataTimeout = dataTimeout
		# last time data was received or successfully sent
		self.lastDataExchangeTime = None

	def setBlocking(self, bBlocking):
		# 0 for timeout of 0 ms (does not hang, waiting for message)
		self.client.setblocking(int(bBlocking))

	def connect(self):
		if not self.bConnected:
			self.lastConnectionAttemptTime = dt.now()
			self.lastDataExchangeTime = dt.now() # Is this necessary?
			try: 
				self.client.connect(self.addr)
				self.bConnected = True
				print "TCP connected on port ", self.port
				return True
			except:
				print "TCP could not connect on port ", self.port
				return False
		else:
			print "TCP is already connected on port ", self.port
			return False

	def close(self):
		if self.bConnected:
			print "TCP closing on port ", self.port
			self.client.close()
			self.bConnected = False
			return True
		else:
			print "TCP socket is already closed, but closing again"
			self.client.close()
			self.bConnected = False
			return True

	# get any waiting messages (attempts to receive messageSize bytes), 
	# return the message
	def getMessage(self, messageSize):
		try:
			message = self.client.recv(messageSize)
			print "TCP received message with ", len(message), " / ", messageSize, " bytes"
			if (len(message) > 0):
				self.lastDataExchangeTime = dt.now()
		except:
			print "TCP did not receive a message"

	# send message; returns number of bytes sent
	def sendMessage(self, message):
		if self.bConnected:
			bytesSent = self.client.send(message)
			if len(message) == 0:
				print "TCP sent message of size zero??"
				return 0
			if bytesSent > 0:
				# data was successfully sent
				self.lastDataExchangeTime = dt.now()
				print "TCP sent ", bytesSent, " byte message: \"", message, "\""
			else:
				# channel is closed (not active)
				self.flagReconnect = True
				print "TCP sent received 0 bytes, so flag a reconnect"
			return bytesSent
		else:
			print "TCP not connected, so message not sent: \"", message, "\""
			return -1

	def updateConnection(self):
		# if flag reconnect has been set, attempt reconnection
		if (self.flagReconnect):
			print "Reconnecting..."
			self.close()
			self.connect()
			return None

		# otherwise, first check connection activity
		if not self.bConnected:
			if (self.lastConnectionAttemptTime is not None):
				diff = dt.now() - self.lastConnectionAttemptTime
				if (diff > self.connectionTimeout):
					print "Reconnecting... via connection timeout"
					# attempt to reconnect
					self.close()
					self.connect()
		else:
			# we're connected, so check data activity
			if (self.lastDataExchangeTime is not None):
				diff = timeNow - self.lastDataExchangeTime
				if (diff > self.dataTimeout):
					print "Reconnecting... via data timeout"
					# we haven't received data in long enough time, so reconnect
					self.close()
					self.connect()

class udpClient:
	def __init__(self, host, port):
		self.host = host
		self.port = port
		self.addr = (host, port)
		# socket.SOCK_DGRAM is UDP communication
		self.client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.lastConnectionTime = None
		# bool to know whether socket is connected
		self.bActive = False 

	def setBlocking(self, bBlocking):
		self.client.setblocking(int(bBlocking))

	def connect(self):
		self.client.bind(self.addr)

	def close(self):
		self.client.close()

	def getMessage(self):



class udpServer:
	def __init__(self, host, port):
		self.host = host
		self.port = port
		self.addr = (host, port)
		self.server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.lastConnectionTime = None
		# bool to know whether socket is connected
		self.bActive = False 

	def close(self):
		self.client.close()

	def sendMessage(self, message):
		bytesSent = self.server.sendto(message, self.addr)
		if (bytesSent > 0):
			self.lastConnectionTime = dt.now()




timeZero = dt.now()

class urServer:
	def __init__(self, tcp_host, tcp_port, udp_host, udp_from_port, udp_to_port):
		self.tcp = tcpClient(tcp_host, tcp_port)
		self.udpFrom = udpClient(udp_host, udp_from_port)
		self.udpTo = udpServer(udp_host, udp_to_port)

	def setup():
		self.tcp.setBlocking(False)
		self.udpFrom.setBlocking(False)

	def connect():






		



# 127.0.0.1 is localhost
# 30003 is the realtime communication port on the UR5
server = urServer("192.168.1.9", 30003, "127.0.0.1", 5001, 5002)



