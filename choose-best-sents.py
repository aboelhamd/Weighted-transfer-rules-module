import sys

if (len(sys.argv) != 6) :
	print('\nUsage: python3 choose-best-sents.py scores-file combinations-file minwer-file minper-file minwerper-file');
	sys.exit()

minwerFile = open(sys.argv[3], 'w+')
minperFile = open(sys.argv[4], 'w+')
minwerperFile = open(sys.argv[5], 'w+')

sents, wers, pers, werspers = [], [], [], []
minwer, minper, minwerper, minwerI, minperI, minwerperI = 10000.,10000.,10000.,0,0,0

with open(sys.argv[1]) as scoresFile, open(sys.argv[2]) as combFile: 
  for scores, sent in zip(scoresFile, combFile):
    #print(scores.strip())
    if (scores.strip()) :
      sents.append(sent)
      scoresArr = list(map(float, scores.split()))
      wer = scoresArr[0]
      per = scoresArr[1]
      werper = wer+per
      
      if (wer < minwer) :
        minwer = wer
        minwerI = len(wers)

      if (per < minper) :
        minper = per
        minperI = len(pers)

      if (werper < minwerper) :
        minwerper = werper
        minwerperI = len(werspers)

      wers.append(wer)
      pers.append(per)
      werspers.append(werper)
    
    else :
      minwerFile.write(sents[minwerI])
      minperFile.write(sents[minperI])
      minwerperFile.write(sents[minwerperI])
      
      minwer, minper, minwerper, minwerI, minperI, minwerperI = 10000.,10000.,10000.,0,0,0
      sents, wers, pers, werspers = [], [], [], []
      

scoresFile.close()
combFile.close()
minwerFile.close()
minperFile.close()
minwerperFile.close()
