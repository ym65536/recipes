#!/usr/bin/env python3
#-*- coding:utf-8 -*-

from socket import *

def main():
  HOST ='localhost'
  PORT = 9981
  BUFFSIZE = 2048
  ADDR = (HOST, PORT)

  client = socket(AF_INET, SOCK_STREAM)
  client.connect(ADDR)
  data = "hello, world" 
  print("input data:" + data)
  client.send(data.encode())
  data = client.recv(BUFFSIZE).decode()
  print(data)
  client.close()

if __name__ == '__main__':
  main()
