import sys

if (len(sys.argv) != 5) :
	print('\nUsage: python3 rem-par-new-lines source-file-path target-file-path new-source-file-path new-target-file-path');
	sys.exit()

file3 = open(sys.argv[3], 'w+')
file4 = open(sys.argv[4], 'w+')

with open(sys.argv[1]) as file1, open(sys.argv[2]) as file2: 
    for line1, line2 in zip(file1, file2):
      line1 = line1.strip()
      line2 = line2.strip()
      if (len(line1)>0 and len(line2)>0):
        file3.write(line1+"\n")
        file4.write(line2+"\n")


file1.close()
file2.close()
file3.close()
file4.close()
