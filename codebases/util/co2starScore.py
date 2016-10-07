import sys
from util import parseSequences

class Parameters:
    def __init__(self):
        self.INSERTION_COST = 1
        self.DELETION_COST = 1
        self.MATCH_COST = 0
        self.MISMATCH_COST = 1

class Statistics:
    def __init__(self):
        self.NUM_INSERTION = 0
        self.NUM_DELETION = 0
        self.NUM_MATCH = 0
        self.NUM_MISMATCH = 0
        self.orgDNA = ""
        self.score = []

    def dumpResult(self):
        print "Original DNA:"
        print " ", self.orgDNA
        print "Pairwise Statistics" 
        print "  NUM_INSERTION:", self.NUM_INSERTION 
        print "  NUM_DELETION:", self.NUM_DELETION
        print "  NUM_MATCH:", self.NUM_MATCH
        print "  NUM_MISMATCH:", self.NUM_MISMATCH
        print "Columnwise Statistics"
        star_score = sum(self.score)
        for i in range(len(self.score)):
            percentage = '%2.1f%%' % (self.score[i] / star_score*100)
            print "  c["+str(i)+"]:", self.score[i], "\t", percentage
        print "Sum of Pair Scores:", star_score

def usage():
    usage_info = '''This program computes the Star score for multiple sequence alignment.
    python co2starScore.py [data_file]
It is assumed that all sequences in the input file are of the same length
Please use Python 2.7 to run this program.'''
    print usage_info

def computePairDNAScore(cid, dna1, dna2, P, S):
    if cid >= len(S.score): S.score.append(0)
    if dna1 == dna2:
        S.NUM_MATCH += 1
        S.score[cid] += P.MATCH_COST
    elif dna1 == '-' and not dna2 == '-':
        S.NUM_INSERTION += 1
        S.score[cid] += P.INSERTION_COST
    elif not dna1 == '-' and dna2 == '-':
        S.NUM_DELETION += 1
        S.score[cid] += P.DELETION_COST
    elif not dna1 == dna2:
        S.NUM_MISMATCH += 1
        S.score[cid] += P.MISMATCH_COST

def computeStarScore(allSeqs, Param, Stats):
    numSeqs = len(allSeqs)
    for i in range(numSeqs):
        for j in range(i+1, numSeqs):
            assert len(allSeqs[i]) == len(allSeqs[j])
    numDNA = len(allSeqs[0])
    for j in range(numDNA):
        votes = {}
        for i in range(numSeqs):
            if allSeqs[i][j] not in votes:
                votes[allSeqs[i][j]] = 1
            else:
                votes[allSeqs[i][j]] += 1
        max_elem = None
        max_freq = -1
        for key in votes.keys():
            if votes[key] > max_freq:
                max_elem = key
                max_freq = votes[key]
        dna1 = max_elem
        Stats.orgDNA += dna1
        for i in range(numSeqs):
            dna2 = allSeqs[i][j]
            computePairDNAScore(j, dna1, dna2, Param, Stats)

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
    computeStarScore(allSeqs, Param, Stats)
    Stats.dumpResult()

if __name__ == '__main__':
    main()
