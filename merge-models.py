from os import listdir
from os.path import isfile, join
import sys

if (len(sys.argv) != 3) :
	print('\nUsage: python merge-models.py modelsdest newfile');
	sys.exit()

modelsdest = sys.argv[1]
newfile = open(sys.argv[2], 'w+')


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
