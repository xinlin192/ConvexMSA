import sys

alphabet = ['A', 'C', 'G', 'T']

def usage():
    usage_info = '''Generate initial model of Profile HMM for ToPS B Welch algorithm. 
        python generate_init_model [Model Length] 
    Note that starting and ending states are exclusive in model length.'''
    print usage_info

def generate_init_model(L):
    model_name = '''model_name = "ProfileHiddenMarkovModel"'''

    state_names = '''state_names = ('''
    for i in range(L+2): state_names += '''"M''' + str(i) + '''",'''
    for i in range(L+1): state_names += '''"I''' + str(i) + '''",'''
    for i in range(1,L): state_names += '''"D''' + str(i) + '''",'''
    state_names += '''"D''' + str(L) + '''")'''
    observation_symbols = '''observation_symbols = ('''
    for i in range(len(alphabet)):
        observation_symbols += '''"'''+alphabet[i] + '''"'''
        if i < len(alphabet) - 1: observation_symbols += ","
        else: observation_symbols += ")"
    transitions = '''transitions = ('''
    for i in range(L):
        transitions += '''"I'''+str(i)+'''" | "M'''+str(i)+'''": 0.33333;\n\t\t'''
        transitions += '''"D'''+str(i+1)+'''" | "M'''+str(i)+'''": 0.33333;\n\t\t'''
        transitions += '''"M'''+str(i+1)+'''" | "M'''+str(i)+'''": 0.33333;\n\t\t'''
        transitions += '''"D'''+str(i+1)+'''" | "I'''+str(i)+'''": 0.33333;\n\t\t'''
        transitions += '''"I'''+str(i)+'''" | "I'''+str(i)+'''": 0.33333;\n\t\t'''
        transitions += '''"M'''+str(i+1)+'''" | "I'''+str(i)+'''": 0.33333;\n\t\t'''
        if i > 0:
            transitions += '''"D'''+str(i+1)+'''" | "D'''+str(i)+'''": 0.33333;\n\t\t'''
            transitions += '''"I'''+str(i)+'''" | "D'''+str(i)+'''": 0.33333;\n\t\t'''
            transitions += '''"M'''+str(i+1)+'''" | "D'''+str(i)+'''": 0.33333;\n\t\t'''
    transitions += '''"I''' + str(L) + '''" | "M''' + str(L) + '''": 0.5;\n\t\t'''
    transitions += '''"M''' + str(L+1) + '''" | "M''' + str(L) + '''": 0.5;\n\t\t'''
    transitions += '''"I''' + str(L) + '''" | "I''' + str(L) + '''": 0.5;\n\t\t'''
    transitions += '''"M''' + str(L+1) + '''" | "I''' + str(L) + '''": 0.5;\n\t\t'''
    transitions += '''"I''' + str(L) + '''" | "D''' + str(L) + '''": 0.5;\n\t\t'''
    transitions += '''"M''' + str(L+1) + '''" | "D''' + str(L) + '''": 0.5;\n\t\t'''
    transitions += '''"M''' + str(L+1) + '''" | "M''' + str(L+1) + '''": 1.0)'''
    emission_probabilities = '''emission_probabilities = ('''
    for i in range(L+1):
        if i > 0:
            for elem in alphabet:
                emission_probabilities += '''"%s" | "M%d": 0.25;\n''' % (elem, i)
        if i < L:
            for elem in alphabet:
                emission_probabilities += '''"%s" | "I%d": 0.25;\n''' % (elem, i)
    for i in range(len(alphabet)):
        emission_probabilities += '''"%s" | "I%d": 0.25''' % (alphabet[i], L)
        if i < len(alphabet) - 1: emission_probabilities += ";\n"
        else: emission_probabilities += ")"
    initial_probabilities = '''initial_probabilities= ("M0": 1.0)'''

    print model_name
    print state_names
    print observation_symbols
    print transitions
    print emission_probabilities
    print initial_probabilities

def main ():
    if len(sys.argv) < 2: 
        usage()
        return -1
    length = int(sys.argv[1])
    generate_init_model(length)

if __name__ == '__main__':
    main()
