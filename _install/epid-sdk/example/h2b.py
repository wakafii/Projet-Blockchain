import binhex as b
import sys

def main():
	argc = len(sys.argv)
	if argc < 3:
		print("Bad Arguments")
		return
	try:
		fp = open(sys.argv[2])
	except IOError:
		# If not exists, create the file
		fp = open(sys.argv[2], 'w+')
		fp.write("")
	finally:
		fp.close()
	b.hexbin(sys.argv[1], sys.argv[2])
		
main()
