# Takes sample.pid and create an easily readable list
input = open("sample.pid",'r')
o = open("index_122.dat",'w')

index = 0
information = {};
algorithm = name = ""
indexs = []

for r in input:
    if (r[0] == "#") :
        continue
    index, algorithm, name = r.split(":")
    if (len(name) and name != "void\n") :
        information[int(index)] = name
        indexs.append(int(index))

indexs.sort()
for i in range(len(indexs)):
    o.write(str(indexs[i])+" "+information[indexs[i]])

input.close()
o.close()
