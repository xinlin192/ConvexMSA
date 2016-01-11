import sys
from util import parseSequences

def usage():
    usage_info = '''This program converts from msa data file to co data file.
    python msa2fasta.py [msa_data_file]
It is assumed that all sequences in the input file are of the same length
Please use Python 2.7 to run this program.'''
 
def main():
    argn = len(sys.argv)
    if argn != 2:
        usage()
        sys.exit()
    seqfile = sys.argv[1]
    allSeqs = parseSequences(seqfile)
    print "Input Sequence File:", seqfile
    for i in range(len(allSeqs)):
        print ">seq"+str(i), str(i)
        print allSeqs[i]
    
if __name__ == "__main__":
    main()
