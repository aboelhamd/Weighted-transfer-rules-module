from os import listdir
from os.path import isfile, join
import sys

if (len(sys.argv) != 4) :
	print('Usage: python merge-models.py modelsdest localeid newfile');
	sys.exit(-1)

newfile = open(sys.argv[3], 'w')
localeid = sys.argv[2]
modelsdest = sys.argv[1]

# localeid
newfile.write("%s\n" % localeid)

models = [f for f in listdir(modelsdest) if isfile(join(modelsdest, f))]

for i in range(len(models)) :
	# model name
	newfile.write("file:%s\n" % models[i])
	model = open(modelsdest+"/"+models[i], 'r')
	# skip the first line	
	model.readline()
	for line in model:
		newfile.write("%s" % line)
	model.close()

newfile.close()
