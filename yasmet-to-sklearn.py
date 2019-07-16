import sys
import re

if (len(sys.argv) != 3) :
	print('\nUsage: python3 yasmet-to-sklearn.py yasmet-dataset sklearn-dataset');
	sys.exit()

yasmet = open(sys.argv[1], 'r')
sklearn = open(sys.argv[2], 'w+')

# first line is number of ambiguous rules (classes)
rules_len = int(yasmet.readline())

# titles for csv file
line = yasmet.readline()
words_len = len(re.sub(r"_[0-9]+:[0-9]+", "", line.split("#",3)[1]).strip().split(" "))
titles = "rule,weight"
for i in range (words_len) :
  titles += ",word" + str(i+1)
sklearn.write(titles+"\n")

while line:
  features = line.split("#",3)[0].replace("$ ", "")
  features += re.sub(r"_[0-9]+:[0-9]+", "", line.split("#",3)[1]).strip()
  sklearn.write(features.replace(" ",",")+"\n")
  line = yasmet.readline()

yasmet.close()
sklearn.close()
