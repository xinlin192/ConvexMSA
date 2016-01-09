import sys

class Parameters:
    def __init__(self):
        self.ONE_GAP_COST = 1.8
        self.MATCH_COST = 0
        self.MISMATCH_COST = 2.2

class Statistics:
    def __init__(self):
        self.NUM_GAP_MATCH = 0
        self.NUM_ONE_GAP = 0
        self.NUM_MATCH = 0
        self.NUM_MISMATCH = 0
        self.NUM_PAIRS = 0
        self.score = []

def usage():
    usage_info = '''This program computes the Sum of Pairs score for multiple sequence alignment.
    python sumPairs.py [data_file]
It is assumed that all sequences in the input file are of the same length
Please use Python 2.7 to run this program.'''
    print usage_info

def parseSequences(fname):
    try:
        f = open(fname)
    except:
        print "IO issue: the specified file might not exist."
        sys.exit(0)
    allSeqs = []
    for line in f:
        allSeqs.append(line.strip('\n'))
    f.close()
    return allSeqs

def computePairwiseScore(seq1, seq2, P, S):
    numDNA = len(seq1)
    assert numDNA == len(seq2) 
    for i in range(numDNA):
        if i >= len(S.score): S.score.append(0)
        dna1 = seq1[i]
        dna2 = seq2[i]
        S.NUM_PAIRS += 1
        if dna1 == '-' and dna2 == '-':
            S.score[i] += P.MATCH_COST
            S.NUM_GAP_MATCH += 1
        elif dna1 == '-' or dna2 == '-':
            S.score[i] += P.ONE_GAP_COST
            S.NUM_ONE_GAP += 1
        elif dna1 == dna2:
            S.score[i] += P.MATCH_COST
            S.NUM_MATCH += 1
        elif not (dna1 == dna2):
            S.score[i] += P.MISMATCH_COST
            S.NUM_MISMATCH += 1

def computeSumPairsScore(allSeqs, Param, Stats):
    numSeqs = len(allSeqs)
    for i in range(numSeqs):
        seq1 = allSeqs[i]
        for j in range(i+1, numSeqs):
            seq2 = allSeqs[j]
            computePairwiseScore(seq1, seq2, Param, Stats)

def main():
    argn = len(sys.argv)
    if argn != 2:
        usage()
        sys.exit()
    seqfile = sys.argv[1]
    allSeqs = parseSequences(seqfile)
    print "Input Sequence File:", seqfile
    for seq in allSeqs:
        print " ", seq
    Param = Parameters()
    Stats = Statistics()
    computeSumPairsScore(allSeqs, Param, Stats)
    print "Pairwise Statistics" 
    print "  NUM_GAP_MATCH:", Stats.NUM_GAP_MATCH 
    print "  NUM_ONE_GAP:", Stats.NUM_ONE_GAP
    print "  NUM_MATCH:", Stats.NUM_MATCH
    print "  NUM_MISMATCH:", Stats.NUM_MISMATCH
    print "  NUM_TOTAL_PAIRS:", Stats.NUM_PAIRS
    print "Columnwise Statistics"
    SOP_score = sum(Stats.score)
    for i in range(len(Stats.score)):
        percentage = '%2.1f%%' % (Stats.score[i] / SOP_score*100)
        print "  c["+str(i)+"]:", Stats.score[i], "\t", percentage
    print "Sum of Pair Scores:", SOP_score

if __name__ == "__main__":
    main()
