import sys

if (len(sys.argv) != 4) :
	print('\nUsage: python rem-bad-sents.py source-file ambig-target-file new-source-file');
	sys.exit()

srcFile = open(sys.argv[1], 'r')
ambigTarFile = open(sys.argv[2], 'r')
newSrcFile = open(sys.argv[3], 'w+')

sents = []

for sent in ambigTarFile: 
  if (sent.strip()) :
    sents.append(sent)

  else :
    src = scrFile.readline()
    for sent in sents :
      if (line.find("#") == -1 and line.find("@") == -1) :
        newSrcFile.write()
    sents.clear()

srcFile.close()
ambigTarFile.close()
newSrcFile.close()
