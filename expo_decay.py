# File: exponential_decay_plot.py

import numpy as np
import matplotlib.pyplot as plt

# Parameters
N0 = 100  # Initial quantity
tau = 2   # Mean lifetime
t_half = tau * np.log(2)  # Half-life

# Time array
t = np.linspace(0, 5 * tau, 500)

# Exponential decay function
N = N0 * np.exp(-t / tau)

# Plot
plt.figure(figsize=(10, 6))
plt.plot(t, N, label=f"$N(t) = N_0 e^{{-t/\\tau}}$", color="blue")
plt.axvline(t_half, color="orange", linestyle="--", label=f"Half-life $t_{{1/2}} = {t_half:.2f}$")
plt.axvline(tau, color="green", linestyle="--", label=f"Mean lifetime $\\tau = {tau:.2f}$")

# Highlight N(t_half) and N(tau)
plt.scatter([t_half, tau], [N0 / 2, N0 / np.e], color=["orange", "green"], zorder=5)
plt.text(t_half, N0 / 2, f"$N(t_{{1/2}}) = {N0 / 2}$", ha="right", color="orange")
plt.text(tau, N0 / np.e, f"$N(\\tau) = {N0 / np.e:.2f}$", ha="right", color="green")

# Labels and grid
plt.title("Exponential Decay with Half-life and Mean Lifetime")
plt.xlabel("Time $t$")
plt.ylabel("Quantity $N(t)$")
plt.legend()
plt.grid(True)
plt.show()
