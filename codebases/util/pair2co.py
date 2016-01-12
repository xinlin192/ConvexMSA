import sys
from util import parseSequences

def usage():
    usage_info = '''This program converts .pair file to .co file for multiple sequence alignment.
    python pair2co.py [data_file]
It is assumed that all sequences in the input file are of the same length
Please use Python 2.7 to run this program.'''
    print usage_info

### Conversion from .pair to .co 
def convertPair2CO (allModelSeqs, allDataSeqs):
    numSeqs = len(allModelSeqs)
    allCOSeqs = ["" for i in range(numSeqs)]
    pos = [0 for i in range(numSeqs)]
    while True:
        insertions_ids = {}
        for i in range(numSeqs):
            if pos[i] >= len(allModelSeqs[i]): continue
            if (allModelSeqs[i][pos[i]] == '-'):
                insertions_ids[i] = True
        if bool(insertions_ids):
            for i in range(numSeqs):
                if i in insertions_ids:
                    if pos[i] >= len(allDataSeqs[i]): continue
                    allCOSeqs[i] += allDataSeqs[i][pos[i]]
                    pos[i] += 1
                else:
                    allCOSeqs[i] += '-'
        else:
            for i in range(numSeqs):
                if pos[i] >= len(allDataSeqs[i]): continue
                allCOSeqs[i] += allDataSeqs[i][pos[i]]
                pos[i] += 1
        terminated = True
        for i in range(numSeqs):
            if (pos[i] != len(allModelSeqs[i])):
                terminated = False
                break
        if terminated: break
    
    return allCOSeqs

def main():
    argn = len(sys.argv)
    if argn != 2:
        usage()
        sys.exit()
    seqfile = sys.argv[1]
    allSeqs = parseSequences(seqfile)
    numSeqs = len(allSeqs)
    allModelSeqs = []
    allDataSeqs = []
    # odd-indexed sequence is model sequence
    for i in range(0, numSeqs,2):
        allModelSeqs.append(allSeqs[i])
    # even-indexed sequence is data sequence
    for i in range(1, numSeqs,2):
        allDataSeqs.append(allSeqs[i])
    allCOSeqs = convertPair2CO (allModelSeqs, allDataSeqs)
    for seq in allCOSeqs:
        print seq
    
if __name__ == "__main__":
    main()
