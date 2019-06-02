import sys

if (len(sys.argv) != 4) :
	print('\nUsage: python3 rem-bad-sents.py source-file ambig-target-file(with new lines) new-source-file');
	sys.exit()

srcFile = open(sys.argv[1], 'r')
ambigTarFile = open(sys.argv[2], 'r')
newSrcFile = open(sys.argv[3], 'w+')

sents = []

for sent in ambigTarFile: 
  if (sent.strip()) :
    sents.append(sent)

  else :
    src = srcFile.readline()
    bad = False    
    for sent in sents :
      if (sent.find("*") > -1 or sent.find("#") > -1 or sent.find("@") > -1) :
        bad = True
        break
    if (not bad) : 
      newSrcFile.write(src)
    sents.clear()

srcFile.close()
ambigTarFile.close()
newSrcFile.close()
