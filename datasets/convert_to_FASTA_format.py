import sys;
import os;

try:
    seq_fpath = sys.argv[1];
except IndexError:
    print('python3 convert_to_FASTA_format.py [seq_file]');
    exit();

output_fpath = seq_fpath+'.fasta';

with open(seq_fpath, 'r') as fp_r:
    with open(output_fpath,'w') as fp_w:
        for i, line in enumerate(fp_r):
            fp_w.write('>seq'+str(i)+' '+str(i)+'\n'+line);
