import perlin_numpy as pn
import numpy as np
# np.random.seed(0)
a = pn.generate_perlin_noise_2d([1, 1000000], [1, 1000000 // 10000])[0] / 2 + 0.5
b = pn.generate_perlin_noise_2d([1, 1000000], [1, 1000000 // 1000000])[0] / 2 + 0.5
t = np.copy(a)
a = a / (a + b)
b = b / (t + b)
print(sum(a < b) / 1000000)