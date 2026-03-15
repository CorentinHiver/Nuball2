import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import matplotlib.ticker as ticker
from scipy.optimize import curve_fit
from scipy.optimize import minimize
from scipy.optimize import differential_evolution
from concurrent.futures import ProcessPoolExecutor
from functools import partial
from tqdm import tqdm

# Definitions :

ps = 1e-12
ns = 1e-9
us = 1e-6
ms = 1e-3
s  = 1e+0

# Options :

fissilityFactor = 50.8
useMetagFissility = False
correctHaliflives = True
#Plot
LABELSIZE=15

# Choose which minimization to perform :
# Always minimize delta at least
CLASSIC=True
MIN_N=True
MIN_N1=False
WO_U = False

outputFolder="/mnt/c/Users/coren/OneDrive/CNRS/Nuball2/Calculations/SystematicPlots/"
if useMetagFissility:
  outputFolder+="Metag/"
elif correctHaliflives:
  outputFolder+="halftCorr/"
else :
  outputFolder+="classic/"

xlabelFissility = str()
if useMetagFissility:
  xlabelFissility = r'$\chi = Z^2 / A \times \left[2(a_s / a_c)\left[1 - \frac{\kappa (N - Z)^2}{A^2}\right]\right]^{-1}$'
else:
  xlabelFissility = r'$\chi = Z^2/(50.8 \times A)$'

# Minimization :

def linear(x, a, b):
  return a * x + b

def quadratic(x, a, b, c):
  return a * x * x + b * x + c

function = linear
# function = quadratic

def auto_time_unit(t_seconds):
  prefixes = {
      -15: "fs",
      -12: "ps",
      -9:  "ns",
      -6:  "µs",
      -3:  "ms",
        0:  "s",
        3:  "ks"
  }

  if t_seconds == 0:
    return 0, "s"

  exponent = int(np.floor(np.log10(abs(t_seconds))))
  eng_exp = 3 * (exponent // 3)

  # clamp to known range
  eng_exp = min(max(eng_exp, -15), 3)

  scaled = t_seconds / 10**eng_exp
  unit = prefixes.get(eng_exp, "s")

  return scaled, unit

elements = {87: "Fr", 88: "Ra", 89: "Ac", 90: "Th", 91: "Pa", 92: "U", 93: "Np", 94: "Pu", 95: "Am", 96: "Cm", 97: "Bk", 98: "Cf"}

colors = {97:'b', 96: 'g', 95: 'r', 94: 'c', 93: 'm', 92: 'y', 91: 'darkgray', 90: 'k', 89: 'mediumseagreen', 88:'darkmagenta', 87:'orange'}
markers = {97:'^', 96: 'o', 95: 'v', 94: 'p', 93: 's', 92: 'd', 91: '>', 90: 's', 89: 'P'}
Znames= ['Bk', 'Cm', 'Am', 'Pu', 'Np', 'U', 'Pa', 'Th', 'Ac', 'Ra', 'Fr']

def isEven(x):
  return x%2 == 0
def areEven(x1, x2):
  return isEven(x1) and isEven(x2)
def isOdd(x):
  return x%2 == 1
def areOdd(x1, x2):
  return isOdd(x1) and isOdd(x2)
def areOddEven(x1, x2):
  return isOdd(x1) and isEven(x2) or isEven(x1) and isOdd(x2)

class Params:
  def __init__(self, delta=0, aN=0, N0=146, aN2 = 0, N02=148, aZ=0, Z0=100, aZ2=0, Z02=90, aBarrierHeight=0):
    self.delta = delta
    self.aN    = aN
    self.N0    = N0
    self.aN2   = aN2
    self.N02   = N02
    self.aZ    = aZ
    self.Z0    = Z0
    self.aZ2   = aZ2
    self.Z02   = Z02
    self.aBarrierHeight = aBarrierHeight
  def __str__(self):
    lines = ["Best fit:"]
    if self.delta != 0:
      lines.append(f"delta = {self.delta}")
    if self.aN != 0:
      lines.append(f"aN = {self.aN}, N0 = {self.N0}")
    if self.aN2 != 0:
      lines.append(f"aN2 = {self.aN2}, N02 = {self.N02}")
    if self.aZ != 0:
      lines.append(f"aZ = {self.aZ}, Z0 = {self.Z0}")
    if self.aZ2 != 0:
      lines.append(f"aZ2 = {self.aZ2}, Z02 = {self.Z02}")
    if not lines:
      return "empty"
    return ", ".join(lines)

a_s = 17.64
a_c = 0.72
kappa = 1.87

def calcFissility(Z, A, factor = fissilityFactor):
  if useMetagFissility:
    return Z**2 / A / (2*a_s/a_c * (1-(kappa*(A-2*Z)**2/A**2)))
  else:
    return Z**2 / A / factor

def estimateHalflifeLog(Z:int, A:int, constants, args:Params, function=function):
  t_corr = function(calcFissility(Z, A), *constants)
  N = A-Z
  if (N % 2 == 1) :
    t_corr += args.delta
  if (Z % 2 == 1) :
    t_corr += args.delta
  t_corr += args.aN * (N-args.N0)**2
  t_corr += args.aN2 * (N-args.N02)**2
  t_corr += args.aZ * (Z-args.Z0)**2
  return t_corr

def estimateHalflife(Z:int, A:int, constants, args:Params, function=function):
  return math.pow(10, estimateHalflifeLog(Z, A, constants, args, function))

def estimateHalflifeStr(Z, A, constants, args:Params, function=function):
  val, unit = auto_time_unit(estimateHalflife(Z, A, constants, args, function))
  return f"{val:.2f} {unit}"


# Declarations :

class Isotope:
  def __init__(self, Z, A, t=1, f_br = 1, t_err=0):
    self.Z = int(Z  )
    self.A = int(A  )
    self.N = int(A-Z)
    self.f_br=f_br
    self.t = float(t) if f_br == 1 else t/f_br
    self.logt = float(math.log10(t))
    self.t_err = t_err
    self.name=str(self.A) + elements[self.Z]
  
  @property
  def fissility(self, factor = fissilityFactor):
    return calcFissility(self.Z, self.A, factor)
  
  def correctHalflifeLog(self, args:Params):
    t = self.logt
    
    if (self.N % 2 == 1) :
      t -= args.delta
    if (self.Z % 2 == 1) :
      t -= args.delta
      
    t -= args.aN * (self.N-args.N0)**2
    t -= args.aN2 * (self.N-args.N02)**2
    t -= args.aZ * (self.Z-args.Z0)**2
  
    return t
  
  def estimateHalflifeLog(self, constants, args:Params):
    return estimateHalflifeLog(self.Z, self.A, constants, args)
  
  def estimateHalflife(self, constants, args:Params):
    return estimateHalflife(self.Z, self.A, constants, args)
  
  def estimateHalflifeStr(self, constants, args:Params):
    return estimateHalflifeStr(self.Z, self.A, constants, args)
  
  def __str__(self):
    return f"{self.name}, fissility={int(self.fissility*100)/100.}"
  
  def __repr__(self):
    return f"{self.name}, fissility={int(self.fissility*100)/100.}"

# Data :

FI = np.array([
# Uranium
  Isotope(92, 235,   5.6 * ( 0.01  if correctHaliflives else 1) * ms),
  Isotope(92, 236, 116   * ns),
  Isotope(92, 238, 195   * ns),
# Neptunium
  Isotope(93, 237,  40   / (1.9e-3 if correctHaliflives else 1) * ns),
# Plutonium
  Isotope(94, 235,  30   * ns), Isotope(94, 236,  37   * ps), Isotope(94, 237, 110   * ns),
  Isotope(94, 238, 600   * ps), Isotope(94, 239,   8   * us), Isotope(94, 240,   3.8 * ns),
  Isotope(94, 241,   2.4 * us), Isotope(94, 242,   3.6 * ns), Isotope(94, 243,  60   * ns),
  Isotope(94, 244, 380   * ps), Isotope(94, 245,  90   * ns),
# Americium
  Isotope(95, 237,   5   * ns), Isotope(95, 238,   3.5 * us), Isotope(95, 239, 160   * ns),
  Isotope(95, 240, 900   * us), Isotope(95, 241,   1.5 * us), Isotope(95, 242,  14   * ms),
  Isotope(95, 243,   5.5 * us), Isotope(95, 244,   1   * ms), Isotope(95, 245, 640   * ns),
  Isotope(95, 246,  73   * us),
# Curium
  Isotope(96, 240,  10   * ps), Isotope(96, 241,  15   * ns), Isotope(96, 242,  50   * ps),
  Isotope(96, 243,  42   * ns), Isotope(96, 245,  13   * ns),
# Berkelium
  Isotope(97, 242, 600   * ns), Isotope(97, 244, 820   * ns), Isotope(97, 245,   2   * ns)
])

indices           = [[symbol, [e.Z == Z                            for e in FI]] for Z, symbol in elements.items()]
indices_evenN     = [[symbol, [e.Z == Z and isEven(e.N)            for e in FI]] for Z, symbol in elements.items()]
indices_oddN      = [[symbol, [e.Z == Z and isOdd(e.N)             for e in FI]] for Z, symbol in elements.items()]
indices_evenZ     = [[symbol, [e.Z == Z and isEven(e.Z)            for e in FI]] for Z, symbol in elements.items()]
indices_oddZ      = [[symbol, [e.Z == Z and isOdd(e.Z)             for e in FI]] for Z, symbol in elements.items()]
indices_oddodd    = [[symbol, [e.Z == Z and areOdd(e.Z, e.N)       for e in FI]] for Z, symbol in elements.items()]
indices_eveneven  = [[symbol, [e.Z == Z and areEven(e.Z, e.N)      for e in FI]] for Z, symbol in elements.items()]
indices_oddeven   = [[symbol, [e.Z == Z and (areOddEven(e.Z, e.N)) for e in FI]] for Z, symbol in elements.items()]

# Vectorization :

fissilities = np.array([e.fissility for e in FI])
As          = np.array([e.A         for e in FI])
Zs          = np.array([e.Z         for e in FI])
Ns          = np.array([e.N         for e in FI])
ts          = np.array([e.t         for e in FI])
log_ts      = np.array([e.logt      for e in FI])
evenN       = np.array([e.N%2 == 0  for e in FI])
oddN        = np.array([e.N%2 == 1  for e in FI])

remove_U = [e.Z != 92 for e in FI]

# Helper functions :

def resetColors():
  plt.gca().set_prop_cycle(None)

def fitFI(function, t_corr, x=fissilities, sigma_t_corr=None):
  popt, pcov = curve_fit(function, x, t_corr, sigma=sigma_t_corr)
  residuals = t_corr - function(x, *popt)
  if sigma_t_corr is not None:
    chisq = np.sum((residuals / sigma_t_corr)**2)
  else:
    chisq = np.sum(residuals**2)
  dof = len(t_corr) - len(popt)  # Degrees of freedom
  reduced_chisq = chisq / dof
  return reduced_chisq  # Return reduced chi-squared

# def plotFI():
#   logt = np.array([e.logt for e in FI])
#   for name, index in indices :
#     if (len(Zs[index]) != 0):
#       plt.plot(fissilities[index], logt[index], linestyle='-', color=colors[Zs[index][0]])
#   for name, index in indices_oddN :
#     if (len(Zs[index]) != 0):
#       Z = Zs[index][0]
#       plt.plot(fissilities[index], logt[index], linestyle = '', color=colors[Z], marker=markers[Z], markerfacecolor='1')
#   for name, index in indices_evenN :
#     if (len(Zs[index]) != 0):
#       Z = Zs[index][0]
#       plt.plot(fissilities[index], logt[index], linestyle = '', color=colors[Z], label=name, marker=markers[Z])
#   plt.legend()

def plot_logt(args:Params, x=fissilities, xlabel='fissility', function=function, plotfit=True, corrected = True, grid = False):
  plt.figure()
  if args==Params():
    corrected = True
  halft = np.array([e.correctHalflifeLog(args) for e in FI])
  for name, index in indices :
    if (len(Zs[index]) != 0):
      Z = Zs[index][0]
      plt.plot(x[index], halft[index], linestyle='-', color=colors[Z])
  for name, index in indices_oddN :
    if (len(Zs[index]) != 0):
      Z = Zs[index][0]
      plt.plot(x[index], halft[index], linestyle = '', color=colors[Z], marker=markers[Z], markerfacecolor='1')
  for name, index in indices_evenN :
    if (len(Zs[index]) != 0):
      Z = Zs[index][0]
      plt.plot(x[index], halft[index], linestyle = '', color=colors[Z], label=name, marker=markers[Z])
    
  if xlabel == 'fissility':
    xlabel = xlabelFissility
  plt.xlabel(xlabel, fontsize=LABELSIZE)
  if corrected:
    plt.ylabel(r'$\ln_{10}(\tau_{fission}^{corr})$', fontsize=LABELSIZE)
  else:
    plt.ylabel(r'$\ln_{10}(\tau_{fission})$', fontsize=LABELSIZE)

  if (plotfit):
    popt, pcov = curve_fit(function, x, halft)
    x_plot = np.linspace(np.min(x), np.max(x))
    t_plot = function(x_plot, *popt)
    plt.plot(x_plot, t_plot)

  plt.plot([], [], marker='', linestyle='', label='--')
  plt.plot([], [], marker='o', markerfacecolor='1', markeredgecolor='b', label='Odd', linestyle='')
  plt.plot([], [], marker='o', markerfacecolor='b', markeredgecolor='b', label='Even', linestyle='')
  plt.legend(loc="upper right")
  fig = plt.gcf()
  fig.set_size_inches(7, 6)
  plt.xticks(fontsize=14)
  plt.yticks(fontsize=14)
  if grid:
    plt.grid() 

def plot_halft_residues(args:Params, x=fissilities, xlabel='fissility', function=linear, diffEvenOdd = False, plotFit = False, plotRange = False, grid = False):
  plt.figure()
  halft = np.array([e.correctHalflifeLog(args) for e in FI])
  popt, pcov = curve_fit(function, fissilities, halft)
  t_fit = function(fissilities, *popt)
  residues = halft-t_fit

  if xlabel == 'fissility':
    xlabel = xlabelFissility
  else:
    ax = plt.gca()
    ax.yaxis.set_major_locator(ticker.MultipleLocator(1))

  if diffEvenOdd:
    for name, index in indices_oddN :
      if (len(Zs[index]) != 0):
        plt.plot(x[index], residues[index], label=name, linestyle='-', color=colors[Zs[index][0]], marker='o')
    for name, index in indices_evenN :
      if (len(Zs[index]) != 0):
        plt.plot(x[index], residues[index], color=colors[Zs[index][0]], linestyle='-', marker='v')
  else:
    for name, index in indices :
      if (len(Zs[index]) != 0):
        Z = Zs[index][0]
        plt.plot(x[index], residues[index], linestyle='-', color=colors[Z])
    for name, index in indices_oddN :
      if (len(Zs[index]) != 0):
        Z = Zs[index][0]
        plt.plot(x[index], residues[index], linestyle = '', color=colors[Z], marker=markers[Z], markerfacecolor='1')
    for name, index in indices_evenN :
      if (len(Zs[index]) != 0):
        Z = Zs[index][0]
        plt.plot(x[index], residues[index], linestyle = '', color=colors[Z], label=name, marker=markers[Z])
    
    plt.plot([], [], marker='', linestyle='', label='--')
    plt.plot([], [], marker='o', markerfacecolor='1', markeredgecolor='b', label='Odd', linestyle='')
    plt.plot([], [], marker='o', markerfacecolor='b', markeredgecolor='b', label='Even', linestyle='')

  x_plot = np.linspace(np.min(x), np.max(x))
  y_plot = np.zeros_like(x_plot)
  if plotFit :
    plt.plot(x_plot, y_plot, color='cyan')
  if plotRange: 
    plt.plot(x_plot, y_plot+1, color='red')
    plt.plot(x_plot, y_plot-1, color='red')
  plt.xlabel(xlabel, fontsize=LABELSIZE)
  plt.ylabel(r'$\ln_{10}(\tau_{fission}^{corr}) - \ln_{10}(\tau_{fission})$', fontsize=LABELSIZE)
  plt.legend(loc="upper right")
  fig = plt.gcf()
  fig.set_size_inches(7, 6)
  plt.xticks(fontsize=14)
  plt.yticks(fontsize=14)
  if grid:
    plt.grid() 
  
def plot_halft_estimate(args:Params, x=fissilities, function=linear):
  plt.figure()
  halft = np.array([e.correctHalflifeLog(args) for e in FI])
  for name, index in indices :
    plt.plot(x[index], halft[index], label=name, linestyle='-', marker='o')
  popt, pcov = curve_fit(function, x, halft)
  x_plot = np.linspace(np.min(x), np.max(x))
  t_plot = function(x_plot, *popt)
  plt.plot(x_plot, t_plot)
  plt.legend()


  plot_logt(args=args, function=function)
  plot_halft_residues(args=args, function=function)
  plot_halft_residues(args=args, x=Ns, xlabel='N', function=function)
  plot_halft_residues(args=args, x=Zs, xlabel='Z', function=function)
  plot_halft_residues(args=args, x=As, xlabel='A', function=function)

  plt.show()


# Minimization:

# Minimize with respect to delta
if CLASSIC and not MIN_N and not MIN_N1 and not WO_U:
  def minimizeFunc(args) :
    delta,= args
    data = np.array([e.correctHalflifeLog(Params(delta=delta)) for e in FI])
    return fitFI(function, data) 
  
  initial_params = [3.0]
  bounds = [(0, 5)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, = result.x
  best_param = Params(delta=best_delta)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}")
  
  popt, pcov = curve_fit(function, fissilities, best_data)
  a, b = popt
  print(f'{a:.2f}x + {b:.2f}')

  
  plot_logt(args=Params(), plotfit=False, corrected=False)
  plt.savefig(outputFolder+"RawHalflives.png", dpi=300)
  plot_logt(args=Params(), plotfit=True, corrected=False)
  plt.savefig(outputFolder+"RawHalflives_fit.png", dpi=300)
  plot_logt(args=best_param)
  plt.savefig(outputFolder+"DeltaHalflives.png", dpi=300)
  # plot_logt(args=best_param, x=Ns, xlabel='N', plotfit=False)
  # plot_halft_residues(args=best_param)
  plot_halft_residues(args=best_param, x=Ns, xlabel='N')
  plt.savefig(outputFolder+"DeltaHalflives_ResiduesVsN.png", dpi=300)
  # plot_halft_residues(args=best_param, x=Zs, xlabel='Z')
  # plot_halft_residues(args=best_param, x=As, xlabel='A')

  plt.show()

# Minimize with respect to delta BUT without U
if CLASSIC and not MIN_N and not MIN_N1 and WO_U:
  def minimizeFunc(args) :
    delta,= args
    data = np.array([e.correctHalflifeLog(Params(delta=delta)) for e in FI[remove_U]])
    return fitFI(function, data, fissilities[remove_U]) 
  
  initial_params = [3.0]
  bounds = [(0, 5)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta=best_delta)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}")

  best_param = Params(delta=best_delta)

  plot_logt(args=Params(delta=0), function=function, plotfit=False)
  plot_logt(args=best_param, function=function)
  plot_logt(args=best_param, x=Ns, xlabel='N', function=function, plotfit=False)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, xlabel='N', function=function)
  plot_halft_residues(args=best_param, x=Zs, xlabel='Z', function=function)
  plot_halft_residues(args=best_param, x=As, xlabel='A', function=function)

  plt.show()
  
# Minimize with respect to N and delta
if CLASSIC and MIN_N and not MIN_N1 and not WO_U:
  def minimizeFunc(args) :
    delta, aN, N0 = args
    params = Params(delta=delta, aN=aN, N0=N0)
    data = np.array([e.correctHalflifeLog(params) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 1., 146]
  bounds = [(0, 5), (-1,1), (140, 150)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0 = result.x
  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, {best_param}")
  
  popt, pcov = curve_fit(function, fissilities, best_data)
  print(popt)

  plot_logt(args=Params(), plotfit=False)
  plt.savefig("/mnt/c/Users/coren/OneDrive/CNRS/Nuball2/Calculations/SystematicPlots/RawHalflives.png", dpi=300)



  plot_logt(args=best_param)
  plt.savefig("/mnt/c/Users/coren/OneDrive/CNRS/Nuball2/Calculations/SystematicPlots/CorrectedHalflives.png", dpi=300)
  plot_halft_residues(args=best_param)
  plot_halft_residues(args=best_param, x=Ns, xlabel="N")
  plt.savefig("/mnt/c/Users/coren/OneDrive/CNRS/Nuball2/Calculations/SystematicPlots/NDeltaHalflives_ResiduesVsN.png", dpi=300)
  plot_halft_residues(args=best_param, x=Zs, xlabel="Z")
  plot_halft_residues(args=best_param, x=As, xlabel="A")
  
  plt.show()

# Minimize with respect to N and delta BUT without Uranium
if CLASSIC and MIN_N and MIN_N1 and WO_U:
  def minimizeFunc(args) :
    delta, aN, N0 = args
    params = Params(delta=delta, aN=aN, N0=N0)
    data = np.array([e.correctHalflifeLog(params) for e in FI[remove_U]])
    return fitFI(function, data, fissilities[remove_U]) 

  initial_params = [3.0, 0., 146]
  bounds = [(0, 5), (-1,1), (140, 150)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0 = result.x
  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}")

  plot_logt(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, xlabel="N", function=function)
  plot_halft_residues(args=best_param, x=Zs, xlabel="Z", function=function)
  plot_halft_residues(args=best_param, x=As, xlabel="A", function=function)
  
  plt.show()

# Minimize with respect to N01, N02 and delta
if CLASSIC and MIN_N and MIN_N1 and not WO_U:
  def minimizeFunc(args) :
    delta, aN, N01, aN2, N02 = args
    params = Params(delta=delta, aN=aN, N0=N01, aN2=aN2, N02=N02)
    data = np.array([e.correctHalflifeLog(params) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 1., 146, 1., 148]
  bounds = [(0, 5), (-1,1), (140, 152), (-1,1), (147, 152)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0, best_NC2, best_N02 = result.x
  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0, aN2=best_NC2, N02=best_N02)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}, aN2 = {best_NC2}, N02={best_N02}")
  
  popt, pcov = curve_fit(function, fissilities, best_data)
  print(popt)

  plot_logt(Params())
  plot_logt(args=best_param)
  plot_halft_residues(args=best_param)
  plot_halft_residues(args=best_param, x=Ns, xlabel="N")
  plot_halft_residues(args=best_param, x=Zs, xlabel="Z")
  plot_halft_residues(args=best_param, x=As, xlabel="A")
  
  plt.show()

# Minimize with respect to N, Z and delta
if False:
  def minimizeFunc(args) :
    delta, aN, N0, aZ, Z0 = args
    params = Params(delta=delta, aN=aN, N0=N0, aZ=aZ, Z0=Z0)
    data = np.array([e.correctHalflifeLog(params) for e in FI])
    # data = np.array([e.correctHalflifeLog(Params(delta=delta, aN=aN, N0=N0, aZ=aZ, Z0=Z0)) for e in FI])
    return fitFI(function, data, fissilities) 

  initial_params = [3.0, 0., 146, 0., 95]
  bounds = [(0, 5), (-1,1), (140, 150), (-1,1), (93, 100)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0, best_ZC, best_Z0 = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}, aZ = {best_ZC}, Z0={best_Z0}")

  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0)

  plot_logt(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, xlabel="N", function=function)
  plot_halft_residues(args=best_param, x=Zs, xlabel="Z", function=function)
  plot_halft_residues(args=best_param, x=As, xlabel="A", function=function)

  plt.show()

# Minimize with respect to N, Z and delta BUT without Uranium
if False:
  def minimizeFunc(args) :
    delta, aN, N0, aZ, Z0 = args
    params = Params(delta=delta, aN=aN, N0=N0, aZ=aZ, Z0=Z0)
    data = np.array([e.correctHalflifeLog(params) for e in FI[remove_U]])
    # data = np.array([e.correctHalflifeLog(Params(delta=delta, aN=aN, N0=N0, aZ=aZ, Z0=Z0)) for e in FI])
    return fitFI(function, data, fissilities[remove_U]) 

  initial_params = [3.0, 0., 146, 0., 95]
  bounds = [(0, 5), (-1,1), (140, 150), (-1,1), (93, 100)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0, best_ZC, best_Z0 = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}, aZ = {best_ZC}, Z0={best_Z0}")

  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0)

  plot_logt(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, xlabel="N", function=function)
  plot_halft_residues(args=best_param, x=Zs, xlabel="Z", function=function)
  plot_halft_residues(args=best_param, x=As, xlabel="A", function=function)

  plt.show()
  
# Minimize with respect to N, Z, Z2 and delta
if False:
  def minimizeFunc(args) :
    delta, aN, N0, aZ, Z0, aZ2, Z02 = args
    data = np.array([e.correctHalflifeLog(Params(delta=delta, aN=aN, N0=N0, aZ=aZ, Z0=Z0, aZ2=aZ2, Z02=Z02)) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 0., 146, 0., 95, 0., 91]
  bounds = [(0, 5), (-1,1), (140, 150), (-1,1), (93, 100), (-1,1), (89, 93)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0, best_ZC, best_Z0, best_ZC2, best_Z02 = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0,  aZ2=best_ZC, Z02=best_Z0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}, aZ = {best_ZC}, Z0={best_Z0}, aZ2 = {best_ZC2}, Z02={best_Z02}")

  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0, aZ2=best_ZC2, Z02=best_Z02)

  plot_logt(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, function=function)
  plot_halft_residues(args=best_param, x=Zs, function=function)
  plot_halft_residues(args=best_param, x=As, function=function)

  plt.show()

# ----- Minimum surface in the N0-Z0 plane ---- #

if False:
  rebin = 2
  Z0s = np.arange(80, 100, rebin)
  N0s = np.arange(130, 160, rebin)
  chi2 = np.zeros((len(N0s), len(Z0s)))
  deltas = np.zeros((len(N0s), len(Z0s)))
  NCs = np.zeros((len(N0s), len(Z0s)))
  ZCs = np.zeros((len(N0s), len(Z0s)))
  initial_params = [3.0, 0.]
  bounds = [(0, 5), (-1, 1)]
  # initial_params = [3.0, 0., 0.]
  # bounds = [(0, 5), (-1, 1), (1e-9, 1)]

  for i, N0 in enumerate(tqdm(N0s, desc="Progress")):
    
    for j, Z0 in enumerate(Z0s):  

      def minimizeFuncLocal(args) :
        delta, aN = args
        params = Params(delta=delta, aN=aN, N0=N0, aZ=0.1, Z0=Z0)
        # delta, aN, aZ = args
        # params = Params(delta=delta, aN=aN, N0=N0, aZ=aZ, Z0=Z0)
        data = np.array([e.correctHalflifeLog(params) for e in FI[remove_U]])
        return fitFI(function, data, fissilities[remove_U]) 

      result = minimize(
      # result = differential_evolution(
          minimizeFuncLocal,
          initial_params,
          bounds=bounds,  # Optional: constraints
          method='L-BFGS-B'  # Choose a method
      )
      best_delta, best_NC = result.x
      # best_delta, best_NC, best_ZC = result.x
      chi2  [i, j] = minimizeFuncLocal(result.x) 
      deltas[i, j] = best_delta
      NCs   [i, j] = best_NC
      # ZCs   [i, j] = best_ZC

  def plotN0Z0(array, title=""):  
    plt.figure()
    array = np.array(array)
    plt.imshow(array, extent=[N0s.min(), N0s.max(), Z0s.min(), Z0s.max()], 
              cmap='viridis', 
              origin='lower', 
              aspect='auto'
              ,norm=LogNorm()
              # ,norm=LogNorm(vmin=array.min(), vmax=array.max())
              )
    plt.colorbar()
    plt.xlabel(r'$N_0$')
    plt.ylabel(r'$Z_0$')
    plt.title(title)

  plotN0Z0(chi2, title="chi2")

  #plot aN NZ
  best_i, best_j = np.unravel_index(
    np.argmin(chi2),          # flat index of the minimum
    chi2.shape                # original shape
  )
  best_N0 = N0s[best_i]
  best_Z0 = Z0s[best_j]
  print(best_N0, best_Z0)
  min_Z0 = chi2.min(axis=0)
  min_N0 = chi2.min(axis=1)
  argvmin_Z0 = chi2.argmin(axis=0)
  argvmin_N0 = chi2.argmin(axis=1)

  print(min_Z0)
  print(min_N0)
  print(argvmin_Z0)
  print(argvmin_N0)

  # plt.plot(Z0s[argvmin_N0], Z0s, color='r')
  # plt.plot(Z0s[argvmin_N0], N0s, color='r')
  # plt.plot(Z0s, N0s, color='r')

  # plt.figure()
  # plt.plot(Z0s, min_Z0)
  # plt.xlabel(r'$Z_0$')
  # plt.ylabel(r'$\chi$')
  # plt.figure()
  # plt.figure()
  # plt.plot(N0s, min_N0)
  # plt.xlabel(r'$N_0$')
  # plt.ylabel(r'$\chi$')
  # plt.figure()


  plotN0Z0(deltas, title="deltas")
  # plotN0Z0(ZCs   , title="ZCs"   )
  plotN0Z0(NCs   , title="NCs"   )


  plt.show()

# ----- Minimum surface in the N0:Delta dimension ---- #

if False:
  rebin = 10
  delta0s = np.arange(0, 6, rebin/(10*(6-0)))
  N0s = np.arange(140, 160, rebin/(10*(160-140)))
  chi2 = np.zeros((len(delta0s), len(N0s)))
  NCs = np.zeros((len(delta0s), len(N0s)))
  
  initial_params = [   0.   ]
  bounds         = [(-1.,1.)]
  
  for i, delta in enumerate(tqdm(delta0s, desc="Progress")):
    for j, N0 in enumerate(N0s):
      
      def minimizeFuncLocal(args) :
        aN = args[0]
        data = np.array([e.correctHalflifeLog(Params(delta=delta, aN=aN, N0=N0)) for e in FI])
        return fitFI(function, data)
      
      result = minimize(
          minimizeFuncLocal,
          initial_params,
          bounds=bounds,
          method='L-BFGS-B' 
      )
      aN = result.x[0]
      chi2[i, j] = minimizeFuncLocal(result.x)
      NCs[i, j] = aN
    
  min_N0   = chi2.min(axis=0)
  min_delta= chi2.min(axis=1)
  argvmin_N0   = chi2.argmin(axis=0)
  argvmin_delta= chi2.argmin(axis=1)
  best_i, best_j = np.unravel_index(
    np.argmin(chi2),          # flat index of the minimum
    chi2.shape                # original shape
  )

  plt.figure()
  plt.plot(N0s, min_N0)
  plt.xlabel(r'$N_0$')
  plt.ylabel(r'$\chi^2$')

  plt.figure()
  plt.plot(delta0s, min_delta)
  plt.xlabel(r'$\delta$')
  plt.ylabel(r'$\chi^2$')


  plt.figure()
  chi2 = np.array(chi2)
  plt.imshow(chi2, extent=[N0s.min(), N0s.max(), delta0s.min(), delta0s.max()], 
            cmap='viridis', 
            origin='lower', 
            aspect='auto'
            # ,norm=LogNorm(vmin=chi2.min(), vmax=chi2.max())
            )
  plt.colorbar()
  plt.title(r'$\chi^2$')
  plt.xlabel(r'$N_0$')
  plt.ylabel(r'$\delta$')
  plt.plot(N0s[argvmin_delta], delta0s, color='r')
  plt.plot(N0s, delta0s[argvmin_N0], color='r')
  # plt.plot(delta0s, N0s[argvmin_delta], color='r')
  
  plt.figure()
  NCs = np.array(NCs)
  plt.imshow(NCs, extent=[N0s.min(), N0s.max(), delta0s.min(), delta0s.max()], 
            cmap='viridis', 
            origin='lower', 
            aspect='auto'
            )
  plt.colorbar()
  plt.title(r'$N_0$ strength')
  plt.xlabel(r'$N_0$')
  plt.ylabel(r'$\delta$')
  plt.show()

# ----- Minimize Z0 after finding delta, N0 and aN ---- #

if False:

  def minimizeFunc(args) :
    delta, aN, N0 = args
    params = Params(delta=delta, aN=aN, N0=N0)
    data = np.array([e.correctHalflifeLog(params) for e in FI])
    return fitFI(function, data, fissilities) 

  initial_params = [3.0, 0.5, 146]
  bounds = [(0, 5), (0,2), (140, 150)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0 = result.x
  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, {best_param}")
  
  plot_logt(args=best_param, function=function)
  plt.title("Before minZ")
  
  rebin = 1/10
  Z0s = np.arange(90, 100, rebin)
  chi2 = np.zeros( len(Z0s))
  ZCs  = np.zeros( len(Z0s))
  
  initial_params = [   0.   ]
  bounds         = [(-1.,1.)]
  
  for i, Z0 in enumerate(tqdm(Z0s, desc="Progress")):
    def minimizeFuncLocal(args) :
      aZ, = args
      param=Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=aZ, Z0=Z0)
      data = np.array([e.correctHalflifeLog(param) for e in FI])
      return fitFI(function, data, fissilities)
  
    result = minimize(
      minimizeFuncLocal,
      initial_params,
      bounds=bounds,
      method='L-BFGS-B' 
    )
    aZ, = result.x
    chi2[i] = minimizeFuncLocal(result.x)
    ZCs[i] = aZ
  
  plt.figure()
  plt.plot(Z0s, chi2)
  plt.title(r'$\chi^2$')
  plt.figure()
  plt.plot(Z0s, ZCs)
  plt.title(r'$Z_{strength}$')

  best_Z0 = Z0s[np.argmin(chi2)]
  best_ZC = ZCs[np.argmin(chi2)]

  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0, aZ=best_ZC, Z0=best_Z0)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, {best_param}")
  # print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}")

  plot_logt(args=best_param, function=function)
  plt.title("After minZ")
  
  popt, pcov = curve_fit(function, fissilities, best_data)
  a, b = popt

  print(f"{a:1f}", f"{b:1f}")

  NsPlot = [
    (97, np.arange(236, 252)), #Bk
    (96, np.arange(235, 250)), #Cm
    (95, np.arange(235, 250)), #Am
    (94, np.arange(233, 250)), #Pu
    (93, np.arange(230, 250)), #Np
    (92, np.arange(230, 250)), #U
    (91, np.arange(226, 248)), #Pa
    (90, np.arange(228, 248)), #Th
    (89, np.arange(225, 248))  #Ac
  ]
  
  plt.figure()
  for e in NsPlot:
    Z = e[0]
    pointsNeven = []
    pointsNodd = []
    fissilities_odd = []
    fissilities_even = []
    A_oddN = []
    A_evenN = []
    Z_oddN = []
    Z_evenN = []
    N_oddN = []
    N_evenN = []
    for A in e[1]:
      N = A-Z
      if N%2 == 0:
        pointsNeven.append(estimateHalflifeLog(Z, A, a, b, best_param))
        fissilities_even.append(calcFissility(Z, A))
        A_evenN.append(A)
        Z_evenN.append(Z)
        N_evenN.append(N)
      else:
        pointsNodd.append(estimateHalflifeLog(Z, A, a, b, best_param))
        fissilities_odd.append(calcFissility(Z, A))
        A_oddN.append(A)
        Z_oddN.append(Z)
        N_oddN.append(N)
    # plt.plot(N_evenN, pointsNeven, color=colors[Z], label = elements[Z])
    # plt.plot(N_oddN, pointsNodd, color=colors[Z])
    # plt.plot(Z_evenN, pointsNeven, color=colors[Z], label = elements[Z])
    # plt.plot(Z_oddN, pointsNodd, color=colors[Z])
    plt.plot(fissilities_even, pointsNeven, color=colors[Z], label = elements[Z])
    plt.plot(fissilities_odd, pointsNodd, color=colors[Z])
    plt.legend()
    # plt.plot(A_evenN, pointsNeven, color=colors[Z])
    # plt.plot(A_oddN, pointsNodd  , color=colors[Z])

  y = np.array([e.logt for e in FI])
  # for name, index in indices :
  #   if (len(Zs[index]) != 0):
  #     plt.scatter(fissilities[index], y[index], color=colors[Zs[index][0]], linestyle='', marker=markers[index])
  for name, index in indices_evenN:
    if (len(Zs[index]) != 0):
      plt.plot(fissilities[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
  for name, index in indices_oddN :
    if (len(Zs[index]) != 0):
      plt.plot(fissilities[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='v')
      # plt.plot(Ns[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
      # plt.plot(As[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
      # plt.plot(Zs[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
  
  # Th230 = Isotope(90,230)
  # Th231 = Isotope(90,231)
  # Th232 = Isotope(90,232)

  # plt.plot(Th230.fissility, estimateHalflifeLog(Th230.Z, Th230.A, a, b, best_param), marker='s', ms=5, color='0')
  # plt.annotate(r'$^{230}$Th', xy = (Th230.fissility, estimateHalflifeLog(Th230.Z, Th230.A, a, b, best_param)))
  # plt.plot(Th231.fissility, estimateHalflifeLog(Th231.Z, Th231.A, a, b, best_param), marker='s', ms=5, color='0')
  # plt.annotate(r'$^{231}$Th', xy = (Th231.fissility, estimateHalflifeLog(Th231.Z, Th231.A, a, b, best_param)))
  # plt.plot(Th232.fissility, estimateHalflifeLog(Th232.Z, Th232.A, a, b, best_param), marker='s', ms=5, color='0')
  # plt.annotate(r'$^{232}$Th', xy = (Th232.fissility, estimateHalflifeLog(Th232.Z, Th232.A, a, b, best_param)))

  # plt.xlabel(xlabelFissility)
  # plt.legend()
  # ax = plt.gca()
  # ax.xaxis.set_major_locator(ticker.MultipleLocator(0.01))
  # ax.yaxis.set_major_locator(ticker.MultipleLocator(1))
  # plt.grid()

  plt.show()

# ----- Minimize Z0 after finding delta, N0 and aN, exploring a bit around N0---- #
# Saddle !!

if False:
  def minimizeFunc(args) :
    delta, aN, N0 = args
    params = Params(delta=delta, aN=aN, N0=N0)
    data = np.array([e.correctHalflifeLog(params) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 1., 146]
  bounds = [(0, 5), (-1,1), (140, 150)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0 = result.x
  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, {best_param}")
  
  rebin = 1/10
  Z0s = np.arange(90, 100, rebin)
  N0s = np.arange(best_N0-3, best_N0+3, rebin)
  chi2 = np.zeros(( len(Z0s), len(N0s)))
  ZCs  = np.zeros(( len(Z0s), len(N0s)))
  
  initial_params = [   0.   ]
  bounds         = [(1e-10,1.)]
  
  for i, Z0 in enumerate(tqdm(Z0s, desc="Progress")):
    for j, N0 in enumerate(N0s):
      def minimizeFuncLocal(args) :
        aZ, = args
        param=Params(delta=best_delta, aN=best_NC, N0=N0, aZ=aZ, Z0=Z0)
        data = np.array([e.correctHalflifeLog(param) for e in FI])
        return fitFI(function, data, fissilities)
    
      result = minimize(
          minimizeFuncLocal,
          initial_params,
          bounds=bounds,
          method='L-BFGS-B' 
      )
      aZ, = result.x
      chi2[i, j] = minimizeFuncLocal(result.x)
      ZCs[i, j] = aZ
  
  plt.figure()
  chi2 = np.array(chi2)
  plt.imshow(chi2, extent=[Z0s.min(), Z0s.max(), N0s.min(), N0s.max()], 
            cmap='viridis', 
            origin='lower', 
            aspect='auto'
            ,norm=LogNorm(vmin=chi2.min(), vmax=chi2.max()/2)
            )
  plt.colorbar()
  plt.xlabel(r'$Z_0$')
  plt.ylabel(r'$N_0$')
  plt.title(r'$\chi^2$')

  plt.figure()
  ZCs = np.array(ZCs)
  plt.imshow(ZCs, extent=[Z0s.min(), Z0s.max(), N0s.min(), N0s.max()], 
            cmap='viridis', 
            origin='lower', 
            aspect='auto'
            ,norm=LogNorm(vmin=ZCs.max()/100, vmax=ZCs.max())
            )
  plt.colorbar()
  plt.title(r'$Z_{strength}$')
  plt.xlabel(r'$Z_0$')
  plt.ylabel(r'$N_0$')
  plt.show()

if True:
  # def minimizeFunc(args) :
  #   delta, aN, N0, aN2, N02 = args
  #   params = Params(delta=delta, aN=aN, N0=N0, aN2=aN2, N02=N02)
  #   data = np.array([e.correctHalflifeLog(params) for e in FI])
  #   return fitFI(function, data, fissilities) 
  def minimizeFunc(args) :
    delta, aN, N0 = args
    params = Params(delta=delta, aN=aN, N0=N0)
    data = np.array([e.correctHalflifeLog(params) for e in FI])
    return fitFI(function, data, fissilities) 

  # initial_params = [3.0, 1., 146, 1., 148]
  # bounds = [(0, 5), (-1,1), (140, 150), (-1,1), (140, 150)]
  initial_params = [3.0, 1., 146]
  bounds = [(0, 5), (-1,1), (140, 150)]

  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  # best_delta, best_NC, best_N0, best_NC2, best_N02 = result.x
  # best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0, aN2=best_NC2, N02=best_N02)
  # best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  # print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}, aN2 = {best_NC2}, N02={best_N02}")
  best_delta, best_NC, best_N0 = result.x
  best_param = Params(delta=best_delta, aN=best_NC, N0=best_N0)
  best_data = np.array([e.correctHalflifeLog(best_param) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, aN = {best_NC}, N0={best_N0}")

  popt, pcov = curve_fit(function, fissilities, best_data)
  print(popt)

  Ra228 = Isotope(88,228)
  print("Ra228", Ra228.estimateHalflifeStr(popt, best_param))

  Ra228 = Isotope(88,228)
  print("Ra228", Ra228.estimateHalflifeStr(popt, best_param))

  Ra230 = Isotope(88,230)
  print("Ra230", Ra230.estimateHalflifeStr(popt, best_param))

  U232 = Isotope(92,232)
  print("U232", U232.estimateHalflifeStr(popt, best_param))

  U233 = Isotope(92,233)
  print("U233", U233.estimateHalflifeStr(popt, best_param))

  U234 = Isotope(92,234)
  print("U234", U234.estimateHalflifeStr(popt, best_param))

  U235 = Isotope(92,235)
  print("U235", U235.estimateHalflifeStr(popt, best_param))

  U236 = Isotope(92,236)
  print("U236", U236.estimateHalflifeStr(popt, best_param))

  U237 = Isotope(92,237)
  print("U237", U237.estimateHalflifeStr(popt, best_param))

  U238 = Isotope(92,238)
  print("U238", U238.estimateHalflifeStr(popt, best_param))

  Th231 = Isotope(90,231)
  print("Th231", Th231.estimateHalflifeStr(popt, best_param))

  Th232 = Isotope(90,232)
  print("Th232", Th232.estimateHalflifeStr(popt, best_param))

  Th233 = Isotope(90,233)
  print("Th233", Th233.estimateHalflifeStr(popt, best_param))
  
  Am242 = Isotope(95,242)
  print("Am242", Am242.estimateHalflifeStr(popt, best_param))

  for e in FI:
    print(e.name, e.estimateHalflifeStr(popt, best_param)) 


  NsPlot = [
    (97, np.arange(236, 252)) #Bk
    ,(96, np.arange(235, 250)) #Cm
    ,(95, np.arange(235, 250)) #Am
    ,(94, np.arange(233, 250)) #Pu
    ,(93, np.arange(230, 250)) #Np
    ,(92, np.arange(230, 250)) #U
    ,(91, np.arange(226, 248)) #Pa
    ,(90, np.arange(228, 248)) #Th
    ,(89, np.arange(225, 248)) #Ac
    ,(88, np.arange(223, 245)) #Ra
    ,(87, np.arange(220, 242)) #Fr
  ]
  i = 0
  plt.figure(1)
  for e in NsPlot:
    Z = e[0]
    pointsNeven = []
    pointsNodd = []
    fissilities_odd = []
    fissilities_even = []
    A_oddN = []
    A_evenN = []
    Z_oddN = []
    Z_evenN = []
    N_oddN = []
    N_evenN = []
    for A in e[1]:
      N = A-Z
      if N%2 == 0:
        pointsNeven.append(estimateHalflifeLog(Z, A, popt, best_param))
        fissilities_even.append(calcFissility(Z, A))
        A_evenN.append(A)
        Z_evenN.append(Z)
        N_evenN.append(N)
      else:
        pointsNodd.append(estimateHalflifeLog(Z, A, popt, best_param))
        fissilities_odd.append(calcFissility(Z, A))
        A_oddN.append(A)
        Z_oddN.append(Z)
        N_oddN.append(N)
    
    # plt.plot(N_evenN, pointsNeven, color=colors[Z], label = Znames[i])
    # plt.plot(N_oddN, pointsNodd, color=colors[Z])
    # plt.plot(Z_evenN, pointsNeven, color=colors[Z], label = Znames[i])
    # plt.plot(Z_oddN, pointsNodd, color=colors[Z])
    plt.plot(fissilities_even, pointsNeven, color=colors[Z], label = Znames[i])
    plt.plot(fissilities_odd, pointsNodd, color=colors[Z])
    # plt.plot(A_evenN, pointsNeven, color=colors[Z])
    # plt.plot(A_oddN, pointsNodd  , color=colors[Z])
    i += 1

  y = np.array([e.logt for e in FI])
  # for name, index in indices :
  #   if (len(Zs[index]) != 0):
  #     plt.scatter(fissilities[index], y[index], color=colors[Zs[index][0]], linestyle='', marker=markers[index])
  for name, index in indices_evenN:
    if (len(Zs[index]) != 0):
      plt.plot(fissilities[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
  for name, index in indices_oddN :
    if (len(Zs[index]) != 0):
      plt.plot(fissilities[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='v')
      # plt.plot(Ns[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
      # plt.plot(As[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
      # plt.plot(Zs[index], y[index], color=colors[Zs[index][0]], linestyle='', marker='o')
  plt.plot(Th231.fissility, estimateHalflifeLog(Th231.Z, Th231.A, popt, best_param), marker='s', ms=5, color='0')
  plt.annotate(r'$^{231}$Th', xy = (Th231.fissility, estimateHalflifeLog(Th231.Z, Th231.A, popt, best_param)))
  plt.plot(Th232.fissility, estimateHalflifeLog(Th232.Z, Th232.A, popt, best_param), marker='s', ms=5, color='0')
  plt.annotate(r'$^{232}$Th', xy = (Th232.fissility, estimateHalflifeLog(Th232.Z, Th232.A, popt, best_param)))
  plt.plot(Th233.fissility, estimateHalflifeLog(Th233.Z, Th233.A, popt, best_param), marker='s', ms=5, color='0')
  plt.annotate(r'$^{233}$Th', xy = (Th233.fissility, estimateHalflifeLog(Th233.Z, Th233.A, popt, best_param)))
 
  plt.xlabel(xlabelFissility)
  plt.legend()
  ax = plt.gca()
  ax.xaxis.set_major_locator(ticker.MultipleLocator(0.01))
  ax.yaxis.set_major_locator(ticker.MultipleLocator(1))
  plt.grid()
  plt.show()()