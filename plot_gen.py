import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

df = pd.read_csv("results.csv")
df.set_index('Test Size', inplace=True)

plt.figure(figsize=(10, 6))
df.plot(ax=plt.gca(), marker='o')

plt.yscale('log')
plt.xscale('log')

plt.title('Execution time of various search algorithms')
plt.xlabel('Test Size (log)')
plt.ylabel('Time (ms)(log)')
plt.grid(True, which="major")
plt.legend(title='Algorithm')

plt.tight_layout()
plt.show()
