import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
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

elements = { 90: "Th", 91: "Pa", 92: "U", 93: "Np", 94: "Pu", 95: "Am", 96: "Cm", 97: "Bk", 98: "Cf"}

class Params:
  def __init__(self, delta0=0, NC=0, N0=146, ZC=0, Z0=100, ZC2=0, Z02=90):
    self.delta0 = delta0
    self.NC     = NC
    self.N0     = N0
    self.ZC     = ZC
    self.Z0     = Z0
    self.ZC2     = ZC2
    self.Z02     = Z02

# Declarations :

class Isotope:
  def __init__(self, Z, A, t, t_err=0):
    self.Z = int(Z  )
    self.A = int(A  )
    self.N = int(A-Z)
    self.t = float(t)
    self.logt = float(math.log10(t))
    self.t_err = t_err
    self.name=str(self.A) + elements[self.Z]
  
  @property
  def fissility(self, factor = 50.8):
    return self.Z**2 / self.A / factor
  
  def correctHalflifeLog(self, args:Params):
    t = self.logt
    
    if (self.N % 2 == 1) :
      t -= args.delta0
    if (self.Z % 2 == 1) :
      t -= args.delta0
      
    t += args.NC * (self.N-args.N0)**2
    # t += args.ZC * (self.Z-args.Z0)**2 + args.ZC2 * (self.Z-args.Z02)**2
    t += args.ZC * (self.Z-args.Z0)**2 if self.Z!=92 else 0 
    
    return t
  
  def estimateHalflife(self, args:Params):
    corr = 0
    if (self.N % 2 == 1) :
      t += args.delta0
    if (self.Z % 2 == 1) :
      t += args.delta0
    t -= args.NC * (self.N-args.N0)**2
    t -= args.ZC * (self.Z-args.Z0)**2 if self.Z!=92 else 0 
    return t**corr
  
  def __str__(self):
    return f"{self.name}, fissility={int(self.fissility*100)/100.}"
  
  def __repr__(self):
    return f"{self.name}, fissility={int(self.fissility*100)/100.}"

# Data :

FI = np.array([
# Uranium
  Isotope(92, 235,   5.6 * ms), 
  Isotope(92, 236, 116   * ns), 
  Isotope(92, 237, 200   * us), 
  Isotope(92, 238, 195   * ns), 
# Neptunium
  Isotope(93, 237,  40   * ns),
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

indices = [[symbol, [e.Z == Z for e in FI]] for Z, symbol in elements.items()]

# Vectorization :

fissilities = np.array([e.fissility for e in FI])
As          = np.array([e.A         for e in FI])
Zs          = np.array([e.Z         for e in FI])
Ns          = np.array([e.N         for e in FI])
ts          = np.array([e.t         for e in FI])
log_ts      = np.array([e.logt      for e in FI])

# Helper functions :

def resetColors():
  plt.gca().set_prop_cycle(None)

def plotFI(FI):
  y = np.array([e.t for e in FI])
  for name, index in indices :
    plt.plot(fissilities[index], y[index], label=name, linestyle='-', marker='o')
  plt.legend()
  plt.show()

def linear(x, a, b):
  return a * x + b

def quadratic(x, a, b, c):
  return a * x * x + b * x + c

def fitFI(function, t_corr, sigma_t_corr=None):
  popt, pcov = curve_fit(function, fissilities, t_corr, sigma=sigma_t_corr)
  residuals = t_corr - function(fissilities, *popt)
  if sigma_t_corr is not None:
      chisq = np.sum((residuals / sigma_t_corr)**2)
  else:
      chisq = np.sum(residuals**2)
  dof = len(t_corr) - len(popt)  # Degrees of freedom
  reduced_chisq = chisq / dof
  return reduced_chisq  # Return reduced chi-squared

# Minimization :

# function = linear
function = quadratic

# Minimize with respect to delta
if True:
  def minimizeFunc(args) :
    delta = args
    data = np.array([e.correctHalflifeLog(Params(delta0=delta)) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 0., 146]
  bounds = [(0, 5), (-1,1), (140, 150)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0 = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta0=best_delta, NC=best_NC, N0=best_N0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, NC = {best_NC}, N0={best_N0}")

  best_param = Params(delta0=best_delta, NC=best_NC, N0=best_N0)
  
def plot_halft(args:Params, x=fissilities, function=linear):
  plt.figure()
  halft = np.array([e.correctHalflifeLog(args) for e in FI])
  for name, index in indices :
    plt.plot(x[index], halft[index], label=name, linestyle='-', marker='o')
  popt, pcov = curve_fit(function, x, halft)
  x_plot = np.linspace(np.min(x), np.max(x))
  t_plot = function(x_plot, *popt)
  plt.plot(x_plot, t_plot)
  plt.legend()

def plot_halft_residues(args:Params, x=fissilities, function=linear):
  plt.figure()
  halft = np.array([e.correctHalflifeLog(args) for e in FI])
  popt, pcov = curve_fit(function, fissilities, halft)
  t_fit = function(fissilities, *popt)
  residues = halft-t_fit
  for name, index in indices :
    plt.plot(x[index], residues[index], label=name, linestyle='-', marker='o')
  x_plot = np.linspace(np.min(x), np.max(x))
  y_plot = np.zeros_like(x_plot)
  plt.plot(x_plot, y_plot, color='cyan')
  plt.plot(x_plot, y_plot+1, color='red')
  plt.plot(x_plot, y_plot-1, color='red')
  plt.legend()
  
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


  plot_halft(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, function=function)
  plot_halft_residues(args=best_param, x=Zs, function=function)
  plot_halft_residues(args=best_param, x=As, function=function)

  plt.show()

# Minimize with respect to N and delta
if False:
  def minimizeFunc(args) :
    delta, NC, N0 = args
    data = np.array([e.correctHalflifeLog(Params(delta0=delta, NC=NC, N0=N0)) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 0., 146]
  bounds = [(0, 5), (-1,1), (140, 150)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0 = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta0=best_delta, NC=best_NC, N0=best_N0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, NC = {best_NC}, N0={best_N0}")

  best_param = Params(delta0=best_delta, NC=best_NC, N0=best_N0)

  plot_halft(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, function=function)
  plot_halft_residues(args=best_param, x=Zs, function=function)
  plot_halft_residues(args=best_param, x=As, function=function)

  plt.show()

# Minimize with respect to N, Z and delta
if False:
  def minimizeFunc(args) :
    delta, NC, N0, ZC, Z0 = args
    data = np.array([e.correctHalflifeLog(Params(delta0=delta, NC=NC, N0=N0, ZC=ZC, Z0=Z0)) for e in FI])
    return fitFI(function, data) 

  initial_params = [3.0, 0., 146, 0., 95]
  bounds = [(0, 5), (-1,1), (140, 150), (-1,1), (93, 100)]
  result = minimize(
      minimizeFunc,
      initial_params,
      bounds=bounds,  
      method='L-BFGS-B' 
  )

  best_delta, best_NC, best_N0, best_ZC, best_Z0 = result.x
  best_data = np.array([e.correctHalflifeLog(Params(delta0=best_delta, NC=best_NC, N0=best_N0, ZC=best_ZC, Z0=best_Z0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, NC = {best_NC}, N0={best_N0}, ZC = {best_ZC}, Z0={best_Z0}")

  best_param = Params(delta0=best_delta, NC=best_NC, N0=best_N0, ZC=best_ZC, Z0=best_Z0)

  plot_halft(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, function=function)
  plot_halft_residues(args=best_param, x=Zs, function=function)
  plot_halft_residues(args=best_param, x=As, function=function)

  plt.show()
  
# Minimize with respect to N, Z, Z2 and delta
if True:
  def minimizeFunc(args) :
    delta, NC, N0, ZC, Z0, ZC2, Z02 = args
    data = np.array([e.correctHalflifeLog(Params(delta0=delta, NC=NC, N0=N0, ZC=ZC, Z0=Z0, ZC2=ZC2, Z02=Z02)) for e in FI])
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
  best_data = np.array([e.correctHalflifeLog(Params(delta0=best_delta, NC=best_NC, N0=best_N0, ZC=best_ZC, Z0=best_Z0,  ZC2=best_ZC, Z02=best_Z0)) for e in FI])
  print(f"Chi2 = {fitFI(function, best_data) }, Best fit: delta = {best_delta}, NC = {best_NC}, N0={best_N0}, ZC = {best_ZC}, Z0={best_Z0}, ZC2 = {best_ZC2}, Z02={best_Z02}")

  best_param = Params(delta0=best_delta, NC=best_NC, N0=best_N0, ZC=best_ZC, Z0=best_Z0, ZC2=best_ZC2, Z02=best_Z02)

  plot_halft(args=best_param, function=function)
  plot_halft_residues(args=best_param, function=function)
  plot_halft_residues(args=best_param, x=Ns, function=function)
  plot_halft_residues(args=best_param, x=Zs, function=function)
  plot_halft_residues(args=best_param, x=As, function=function)

  plt.show()

# ----- Minimum surface in the N0-Z0 plane ---- #

if False:
  Z0s = np.linspace(85, 100, 15)
  N0s = np.linspace(140, 160, 20)
  chi2 = np.zeros((len(Z0s), len(N0s)))
  deltas = np.zeros((len(Z0s), len(N0s)))
  NCs = np.zeros((len(Z0s), len(N0s)))
  ZCs = np.zeros((len(Z0s), len(N0s)))
  initial_params = [3.0, 0., 0.]
  bounds = [(0, 5), (-1, 1), (-1, 1)]

  for i, Z0 in enumerate(Z0s):
    print("\r", " " * 50, "\r", int(i/len(Z0s)*100), "%", end="", flush=True)
    
    for j, N0 in enumerate(N0s):  

      def minimizeFuncLocal(args) :
        delta, NC, ZC = args
        data = np.array([e.correctHalflifeLog(Params(delta0=delta, NC=NC, N0=N0, ZC=ZC, Z0=Z0)) for e in FI])
        return fitFI(function, data) 

      result = differential_evolution(
          minimizeFuncLocal,
          # initial_params,
          bounds=bounds,  # Optional: constraints
          # method='L-BFGS-B'  # Choose a method
      )
      best_delta, best_NC, best_ZC = result.x
      chi2  [i, j] = minimizeFuncLocal(result.x) 
      deltas[i, j] = best_delta
      NCs   [i, j] = best_NC
      ZCs   [i, j] = best_ZC

  def plotN0Z0(i, array, title=""):  
    plt.figure(i)
    array = np.array(array)
    plt.imshow(array, extent=[Z0s.min(), Z0s.max(), N0s.min(), N0s.max()], 
              cmap='viridis', 
              origin='lower', 
              aspect='auto')
    plt.colorbar()
    plt.title(title)

  plotN0Z0(1, chi2  , title="chi2"  )
  plotN0Z0(2, deltas, title="deltas")
  plotN0Z0(3, ZCs   , title="ZCs"   )
  plotN0Z0(4, NCs   , title="NCs"   )

  plt.show()

# ----- Minimum surface in the N0:Delta dimension ---- #

if True:
  delta0s = np.linspace(0, 6, 60)
  N0s = np.linspace(130, 160, 300)
  chi2 = np.zeros((len(delta0s), len(N0s)))
  NCs = np.zeros((len(delta0s), len(N0s)))
  
  initial_params = [   0.   ]
  bounds         = [(-1.,1.)]
  
  for i, delta in enumerate(tqdm(delta0s, desc="Progress")):
    for j, N0 in enumerate(N0s):
      
      def minimizeFuncLocal(args) :
        NC = args[0]
        data = np.array([e.correctHalflifeLog(Params(delta0=delta, NC=NC, N0=N0)) for e in FI])
        return fitFI(function, data)
      
      result = minimize(
          minimizeFuncLocal,
          initial_params,
          bounds=bounds,
          method='L-BFGS-B' 
      )
      NC = result.x[0]
      chi2[i, j] = minimizeFuncLocal(result.x)
      NCs[i, j] = NC
    
  plt.figure()
  chi2 = np.array(chi2)
  plt.imshow(chi2, extent=[N0s.min(), N0s.max(), delta0s.min(), delta0s.max()], 
            cmap='viridis', 
            origin='lower', 
            aspect='auto',
            norm=LogNorm(vmin=chi2.min(), vmax=chi2.max())
            )
  plt.colorbar()
  plt.title("chi2")
  plt.scatter(best_N0, best_delta, color='red')
  
  plt.figure()
  NCs = np.array(NCs)
  plt.imshow(NCs, extent=[N0s.min(), N0s.max(), delta0s.min(), delta0s.max()], 
            cmap='viridis', 
            origin='lower', 
            aspect='auto'
            )
  plt.colorbar()
  plt.title("NCs")

plt.show()