import sys

def usage():
    usage_info = '''This program convert .fasta file to .tops file, used for ToPS library.
    python fasta2tops.py [.fasta File] '''

def fasta2tops(allNames, allSeqs):
    nSeqs = len(allSeqs)
    for i in range(nSeqs):
        tmp_str = allNames[i].replace(" ", "_") + ": "
        for j in range(len(allSeqs[i])):
            tmp_str += allSeqs[i][j] + " "
        print tmp_str

def main():
    if len(sys.argv) < 2:
        usage()
        return
    fname = sys.argv[1]

    allNames = []
    allSequences = []
    i = 0
    fp = open(fname)
    for line in fp:
        if i % 2 == 0: allNames.append(line.strip(">\n"))
        else: allSequences.append(line.strip("\n"))
        i += 1
    fasta2tops(allNames, allSequences)
    fp.close()
    

if __name__ == '__main__':
    main()
