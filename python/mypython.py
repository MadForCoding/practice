#!/usr/local/bin/python3
import random
import os
import io

# Random produce the file name
def randomChar():
	str=""
	for i in range(10):
		str += chr(random.randint(97,122))
	str += "\n"
	return str
# calculate the product of number
def product():
	number1 = random.randint(1,42)
	number2 = random.randint(1,42)
	return number1,number2,number1 * number2

str1 = randomChar()
str2 = randomChar()
str3 = randomChar()
with open("file1",'w') as file1:
	file1.write(str1)

with open("file2",'w') as file2:
	file2.write(str2)

with open("file3",'w') as file3:
	file3.write(str3)

#print the file name
print(str1 + str2 + str3, end='')
#print the result of calculation
print ("{0[0]}\n{0[1]}\n{0[2]}".format(product()))

