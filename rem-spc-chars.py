import re
import sys

if (len(sys.argv) != 3) :
	print('\nUsage: python rem-spc-chars.py oldFile newFile');
	sys.exit()

oldfile = open(sys.argv[1], 'r')
newfile = open(sys.argv[2], 'w+')

for line in oldfile :
  line = re.sub('[\\\(\)\[\]\{\}\<\>\|\$\/\'\"]', '', line)
  newfile.write("%s" % line)

oldfile.close()
newfile.close()
