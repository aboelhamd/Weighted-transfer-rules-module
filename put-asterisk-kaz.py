import sys

if (len(sys.argv) != 3) :
	print('\nUsage: python3 put-asterisk-kaz.py text-file new-text-file');
	sys.exit()

oldfile = open(sys.argv[1], 'r')
newfile = open(sys.argv[2], 'w+')

kazakhAlphabet = 'АӘБВГҒДЕЁЖЗИИЙКҚЛМНҢОӨПРСТУҰҮФХҺЦЧШЩЪЫІЬЭЮЯаәбвгғдеёжзийкқлмнңоөпрстуұүфхһцчшщъыыіьэюя'

for sentence in oldfile :
  words = sentence.split()
  for i in range(len(words)) :
    for j in range(len(kazakhAlphabet)) :
      if (kazakhAlphabet[j] in words[i]) :
        words[i] = '*'+words[i]
        break
  sentence = ' '.join(words)
  newfile.write("%s\n" % sentence)

oldfile.close()
newfile.close()
