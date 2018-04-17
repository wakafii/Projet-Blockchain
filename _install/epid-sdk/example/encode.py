import sys
import base64 as b
if len(sys.argv) < 3:
	print("Bad Args")
with open(sys.argv[1], 'r') as input_file:
	with open(sys.argv[2], 'wb+') as output_file:
		b.encode(input_file, output_file)
