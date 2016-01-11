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
