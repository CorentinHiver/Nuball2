import pandas as pd

ref = pd.read_csv("../../ID/index_122.dat", header=None, sep=" ")
i_to_name = {}
i_to_label = {}
for i in range (ref.shape[0]):
    print(i,ref.at[i,0])
    i_to_label[i] = ref.at[i,0]
    i_to_name[ref.at[i,0]] = ref.at[i,1]

calib = pd.read_csv("152Eu_test.dat", sep=" ", header = None)
label_to_a = {}
label_to_b = {}
for i in range (calib.shape[0]):
    label_to_a[calib.at[i,0]] = calib.at[i,2]
    label_to_b[calib.at[i,0]] = calib.at[i,1]

prefix="FParamCalc;"
ADC="_ADC*"

outname = str(input("Name of the output (default : 'converted.txt')"))
if (outname == '') :
    outname = "converted.txt"
o = open(outname,'w')
a = b = 0.
label = 0

for i in range(len(i_to_name)):
    label = i_to_label[i]
    if label in label_to_a.keys():
        print (i,label)
        b = str(label_to_b[label])
        if b[0] != "-" :
             b = "+"+b
        a = str(label_to_a[label])
    else:
        a = "1."
        b = "+0."

    if label in i_to_name:
        line = prefix+i_to_name[label]+";"+i_to_name[label]+ADC+a+b
        o.write(line+"\n")

o.close()
