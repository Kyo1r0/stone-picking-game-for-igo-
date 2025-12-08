import matplotlib.pyplot as plt



# Board size N = 3〜17
N = list(range(3, 18))

# Baseline (何もなし)  from turn1file0
nodes_base = [
    9, 22, 65, 168, 446, 1227, 3313,
    8978, 24480, 66850, 182555, 499304,
    1367593, 3748064, 10280397
]
time_base = [
    0.026104, 0.000162, 0.000436, 0.001214, 0.002374,
    0.00726, 0.023661, 0.074085, 0.222423, 0.695986,
    2.06325, 6.11876, 17.8007, 54.2708, 165.279
]

# Zobrist + Symmetry  from turn1file1
nodes_sym = [
    6, 12, 35, 87, 231, 619, 1676,
    4507, 12289, 33475, 91413, 249774,
    684145, 1874358, 5141141
]
time_sym = [
    0.002336, 0.000219, 0.000218, 0.000428, 0.00123,
    0.003889, 0.013716, 0.035461, 0.104181, 0.281596,
    0.846008, 2.92944, 7.7719, 23.1926, 65.3181
]

# Zobrist only  from turn1file2
nodes_zob = [
    9, 22, 65, 168, 446, 1227, 3313,
    8978, 24480, 66850, 182555, 499304,
    1367593, 3748064, 10280397
]
time_zob = [
    0.040707, 0.000168, 0.000448, 0.001287, 0.00383,
    0.010328, 0.023784, 0.089525, 0.284369, 0.61872,
    2.02883, 5.75761, 18.0294, 53.2567, 203.356
]



plt.figure(figsize=(10, 6))
plt.plot(N, nodes_base, marker='o', label='Baseline')
plt.plot(N, nodes_zob, marker='o', label='Zobrist-only')
plt.plot(N, nodes_sym, marker='o', label='Zobrist + Symmetry')

plt.title("Nodes Explored Comparison")
plt.xlabel("Board Size N")
plt.ylabel("Nodes")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("nodes_comparison.png", dpi=300)
plt.show()



plt.figure(figsize=(10, 6))
plt.plot(N, time_base, marker='o', label='Baseline')
plt.plot(N, time_zob, marker='o', label='Zobrist-only')
plt.plot(N, time_sym, marker='o', label='Zobrist + Symmetry')

plt.yscale('log')
plt.title("Solve Time Comparison (log scale)")
plt.xlabel("Board Size N")
plt.ylabel("Time (seconds, log scale)")
plt.legend()
plt.grid(True, which='both')
plt.tight_layout()
plt.savefig("time_comparison.png", dpi=300)
plt.show()
