from matplotlib import pyplot as plt
import numpy as np

cnt = {}

with open("tests/275") as file:
    lines = [list(map(int, line.split())) for line in file]

n, m = lines[1]
print(n, m)
data = np.array(lines[2:])
# data = data[100]

# data = [len(set(line)) for line in data]

# counter = 0
# c = data[0]
# res = []
# for x in data:
#     if x == c:
#         counter += 1
#     else:
#         res.append(counter)
#         counter = 1
#         c = x
# data = res

def mp(data):
    return [2 * x if x < 128 else 2 * (255 - x) + 1 for x in data]

def da(data):
    return [data[0]] + [(data[i] - data[i - 1] + 512) % 256 for i in range(1, len(data))]

def pa(data):
    res = [data[0]]
    for i in range(1, len(data)):
        res.append((res[i - 1] + data[i] + 512) % 256)
    return res

def xda(data, k = 1):
    return data[:k] + [data[i] ^ data[i - k] for i in range(k, len(data))]

def xpa(data):
    res = [data[0]]
    for i in range(1, len(data)):
        res.append(res[i - 1] ^ data[i])
    return res

def calc_haffman(data):
    count = [0] * 256
    for x in data:
        # count[x % 16] += 1
        # count[x // 16] += 1
        count[x] += 1
    qa, qb = sorted(sorted(count)), []
    ia, ib = 0, 0
    s = 0
    while len(qa) - ia + len(qb) - ib > 1:
        if ib == len(qb) or (ia < len(qa) and qa[ia] < qb[ib]):
            x = qa[ia]
            ia += 1
        else:
            x = qb[ib]
            ib += 1
        if ib == len(qb) or (ia < len(qa) and qa[ia] < qb[ib]):
            y = qa[ia]
            ia += 1
        else:
            y = qb[ib]
            ib += 1
        s += x + y
        qb.append(x + y)
    return s

# print(calc_haffman([1, 1, 3, 6, 7, 10]))

print(sum(calc_haffman(line) for line in data))
print(calc_haffman(data.flatten()))

exit(0)

# data = mp(data)

# print(calc_haffman(data))

print(np.mean(data))

for x in data:
    cnt[x] = cnt.get(x, 0) + 1

x_values = list(cnt.keys())
y_values = np.array(list(cnt.values())) / len(data)

plt.bar(x_values, y_values, width=0.8, color='b')
plt.grid()
# plt.show()
plt.savefig("plot.png")
