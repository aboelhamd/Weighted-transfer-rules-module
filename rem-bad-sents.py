import sys

if (len(sys.argv) != 6) :
	print('\nUsage: python3 rem-bad-sents.py source-file parallel-target-file ambigous-target-file(with new lines) new-source-file new-parallel-target-file');
	sys.exit()

srcFile = open(sys.argv[1], 'r')
trgFile = open(sys.argv[2], 'r')
ambigTrgFile = open(sys.argv[3], 'r')
newSrcFile = open(sys.argv[4], 'w+')
newTrgFile = open(sys.argv[5], 'w+')

sents = []

for sent in ambigTrgFile: 
  if (sent.strip()) :
    sents.append(sent)

  else :
    src = srcFile.readline()
    trg = trgFile.readline()
    bad = False    
    for sent in sents :
      if (sent.find("*") > -1 or sent.find("#") > -1 or sent.find("@") > -1) :
        bad = True
        break
    if (not bad) : 
      newSrcFile.write(src)
      newTrgFile.write(trg)
    sents.clear()

srcFile.close()
trgFile.close()
ambigTrgFile.close()
newSrcFile.close()
newTrgFile.close()
