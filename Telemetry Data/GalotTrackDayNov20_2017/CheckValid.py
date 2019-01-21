import time
import pandas as pd
import matplotlib.pylab as plt
import statsmodels.api as sm

start_time = time.time()

delta = 10

df = pd.read_table('DATA2_rolling.TXT', sep=r"\s+", names=['voltage', 'current', 'power', 'speed',
                                                   'energy', 'distance', 'temperature', 'batteryStatus',
                                                   'batteryVoltage', 'ms'])
df['a'] = 0.0

for index, row in df.iterrows():
    if (index - delta >= 0) and (index + delta < len(df.index)):
        df.loc[index, 'a'] = (df.loc[index + delta, 'speed'] - df.loc[index - delta, 'speed']) / (
            (df.loc[index + delta, 'ms'] -
             df.loc[index - delta, 'ms']) / 1000.0)

df = df[((df['speed'] > -0.0001) & (df['speed'] < 15.0))
        & ((df['a'] < 3.0) & (df['a'] > -1.0))
        & (((df['ms'] >= 415385) & (df['ms'] <= 608300)) | ((df['ms'] >= 700000) & (df['ms'] <= 826951)))]

df = df.reset_index(drop=True)
lowess = sm.nonparametric.lowess(df['a'], df['ms'], frac=0.005)

fig, ax1 = plt.subplots()
ax1.plot(df['ms'], df['a'], color='C0')
ax1.set_xlabel('time (ms)')
ax1.set_ylabel('acceleration')
ax1.tick_params('y', color='C0')

ax2 = ax1.twinx()
ax2.scatter(df['ms'], df['speed'], color='C1', s=5 ** 2)
ax2.set_ylabel('speed')
ax2.tick_params('y', color='C1')


ax3 = ax1.twinx()
ax3.scatter(df['ms'], df['power'], color='C3', s=5 ** 2)
ax3.set_ylabel('power')
ax3.tick_params('y', color='C3')
fig.tight_layout()

run2 = df[(((df['ms'] >= 415385) & (df['ms'] <= 608300)) & ((df['speed'] <= 9.0) & (df['speed'] >= 7.25)))]
run2 = run2.reset_index(drop=True)
speed_vs_a_lowess = sm.nonparametric.lowess(run2['a'], run2['speed'], frac=0.15)
plt.figure(2)
plt.plot(speed_vs_a_lowess[:, 0], speed_vs_a_lowess[:, 1], color='C0')
plt.scatter(run2['speed'], run2['a'], color='C1', s=5 ** 2)
print(time.time() - start_time)
plt.show()
