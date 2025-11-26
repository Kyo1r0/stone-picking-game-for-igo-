import matplotlib.pyplot as plt

N = list(range(3,18))

nodes = [
     9,
    22,
    65,
    168,
    446,
    1227,
    3313,
    8978,
    24480,
    66850,
    182555,
    499304,
    1367593,
    3748064,
    10280397
]


plt.figure(figsize=(10, 6))

plt.plot(N, nodes, marker='o')

plt.title("Number of Nodes Explored")
plt.xlabel("Board Size N")
plt.ylabel("Nodes Explored")
plt.grid(True)

plt.tight_layout()

plt.savefig("cnt_num.png", dpi=300)
plt.show()


plt.figure(figsize=(10, 6))
plt.plot(N, nodes, marker='o')
plt.yscale('log')
plt.title("Nodes Explored (Log Scale)")
plt.xlabel("Board Size N")
plt.ylabel("Nodes (log scale)")
plt.grid(True, which='both')
plt.tight_layout()

# PNG 保存（ログ）
plt.savefig("cnt_num_log.png", dpi=300)