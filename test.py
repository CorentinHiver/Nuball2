import numpy as np
import matplotlib.pyplot as plt
from numpy.polynomial import Polynomial

# Original polynomial coefficients (for example, y = 2x^2 + 3x + 1)
original_coeffs = [1, 3, 2]

# Generate x values
x_values = np.linspace(-10, 10, 100)

# Calculate corresponding y values using the original polynomial
y_values = Polynomial(original_coeffs)(x_values)

# Swap the axes to get (y, x) points
swapped_points = np.array(list(zip(y_values, x_values)))

# Fit a new polynomial to the swapped data points
# We'll use a polynomial of the same degree as the original for simplicity
inverse_poly_refined = Polynomial.fit(swapped_points[:, 0], swapped_points[:, 1], deg=len(original_coeffs) - 1)

# Use the inverse polynomial to generate x values from y values
y_range_refined = np.linspace(min(y_values), max(y_values), 100)
x_inverse_refined = inverse_poly_refined(y_range_refined)

# Fit a polynomial to the (x_inverse_refined, y_range_refined) points to get back the original polynomial
reconstructed_poly_refined = Polynomial.fit(x_inverse_refined, y_range_refined, deg=len(original_coeffs) - 1)

# Calculate the reconstructed y values using the refined reconstructed polynomial
y_reconstructed_refined = reconstructed_poly_refined(x_values)

# Plot the original and refined reconstructed polynomials
plt.figure(figsize=(12, 6))

# Original polynomial
plt.subplot(1, 2, 1)
plt.plot(x_values, y_values, label='Original: y = 2x^2 + 3x + 1', color='blue')
plt.plot(x_values, y_reconstructed_refined, label='Refined Reconstructed', linestyle='--', color='red')
plt.xlabel('x')
plt.ylabel('y')
plt.title('Original vs Refined Reconstructed Polynomial')
plt.legend()

# Difference between original and refined reconstructed polynomials
plt.subplot(1, 2, 2)
plt.plot(x_values, y_values - y_reconstructed_refined, label='Difference', color='purple')
plt.xlabel('x')
plt.ylabel('Difference')
plt.title('Difference Between Original and Refined Reconstructed')
plt.legend()

plt.tight_layout()
plt.show()

# Output the coefficients of the refined reconstructed polynomial
reconstructed_poly_refined.coef
