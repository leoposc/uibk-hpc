import pandas as pd 
import seaborn as sns
import matplotlib.pyplot as plt

df = pd.read_csv('ex02/data.csv',sep=',', header=None)
df.columns = ['ranks', 'yield_when_idle', 'energy', 'time']
convert_dict = {'energy': float,
                'time': float}

df['power'] = df['energy'] / df['time']
df.astype(convert_dict)
plt.xlabel("Ranks")
plt.ylabel("Power")
sns.violinplot(data=df, x="ranks", y="power", hue="yield_when_idle")
plt.axvline(x=0.5, color = 'green', label = 'physical cores', ls="--")
plt.axvline(x=1.5, color = 'red', label = 'total cores', ls="--")
plt.legend()
plt.title("Comparison power consumption for varying ranks")
plt.show()