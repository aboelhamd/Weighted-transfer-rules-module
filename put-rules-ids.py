import sys

if (len(sys.argv) < 3) :
	print('Usage: python put-ids.py original_transfer_file new_transfer_file');
	sys.exit(-1)

oldfile = open(sys.argv[1], 'r')
newfile = open(sys.argv[2], 'w')

id = 0

for line in oldfile :
	if (line.find("rule ") > -1) :
		id = id + 1
	line = line.replace("rule ","rule id=\""+str(id)+"\" ")
	newfile.write("%s\n" % line)

oldfile.close()
newfile.close()
