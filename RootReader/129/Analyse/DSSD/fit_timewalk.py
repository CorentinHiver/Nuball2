def timewalk(Q, a, b, t0, factor = 1):
	y = factor*(t0+a/np.sqrt(Q+b))
	return y

# Load modules :
import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# Paths managing :
Path = "gate642_prompt/timewalk_data/"
fit_path = Path+"png/"
outFile = Path+"fit.fit"
if (not os.path.exists(Path)) : os.mkdir(Path)
if (not os.path.exists(fit_path)) : os.mkdir(fit_path)

# Other parameters :
minE = 500

with open(outFile ,'w') as fOut:

	fOut.write("Function: timewalk (Q,a,b,t0,factor=1) = factor*(t0+a/sqrt(Q+b))\n")
	fOut.write("Settings: minE= "+str(minE)+"\n")
	fOut.write("Parameters: Q a b t0 factor\n")
	for i in range (0,55):
		file = Path+"timewalk_"+str(800+i)+".tw"
		if (not os.path.exists(file)) :
			continue
		with open(file ,'r') as f:
			data = pd.read_csv(f,sep = ' ', names=["Energy","Time"])
			if (len(data)<5):
				continue
			maskData = []
			with open('good_range.list') as info:
				for e in info:	
					if (e.split(' ')[0] == str(800+i)):
						maskData.append(float(e.split(' ')[1]))
						maskData.append(float(e.split(' ')[2]))
			mask = (data.Energy >= maskData[0]) & (data.Energy <= maskData[1])
			Erange = data.Energy[mask]
			Trange = data.Time[mask]

			# Fitting :
			p0 = np.array((1500., -minE+11. , -15. ,  1  ))
			bounds =     ([1000., -minE+10., -100., -1.5], #add 10 to Emin in case parameter = Emin : the function diverges at this value when later used 
				            [3000,   1500 , -5   ,  1.2])
			popt,pcov = curve_fit(timewalk, Erange, Trange, p0=p0, bounds=bounds, method='trf')

			# Plot managing :
			plt.close('all')
			plt.plot(data.Energy[(data.Energy >= minE+10)], timewalk(data.Energy[(data.Energy >= minE+10)],*popt), color='r')
			plt.scatter(data.Energy[(data.Energy >= minE+10)],data.Time[(data.Energy >= minE+10)])
			# plt.plot(1/np.sqrt(data.Energy),timewalk(data,.Energy,*popt), color='r')
			# plt.scatter(1/np.sqrt(data.Energy),data.Time)
			# plt.plot(1/np.sqrt(data.Energy),data.Time - timewalk(data.Energy,*popt), color='r')
			# plt.plot(1/np.sqrt(Erange),Trange - timewalk(Erange,*popt), color='b')
			plt.savefig(fit_path+"fit_"+str(800+i)+".png")

			# Some printing :
			popt = popt.round(3)
			print(800+i, *popt)
			# Data writting :
			fOut.write(str(i+800) + " " + str(popt[0]) + " " + str(popt[1]) + " " + str(popt[2]) + " " + str(popt[3])+"\n")

	fOut.write("end")
	print("Plots written to ", fit_path)
	print("Fitted parameters written to ", outFile)