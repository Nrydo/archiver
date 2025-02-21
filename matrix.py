from matplotlib import pyplot as plt
import numpy as np

cnt = {}

with open("tests/70") as file:
    lines = [list(map(int, line.split())) for line in file]

n, m = lines[1]
print(n, m)
data = np.array(lines[2:])
data -= np.min(data)

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
    return [255 - 2 * x if x < 128 else 2 * x - 256 for x in data]

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

total = 0

def calc_haffman(data):
    global total
    total += len(data)
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
    return (s + 10 * sum(x > 0 for x in count) - 1 + 7) // 8

# k = 128
# print(sum(calc_haffman(data[i:i + k].flatten()) for i in range(0, len(data), k)))

s = 0
cur = data[0]
st = set(cur)
for i in range(1, n):
    ds = set(data[i])
    if ds < st or len(un := ds | st) < 250:
        cur = np.hstack((cur, data[i]))
        st = un
    else:
        s += calc_haffman(cur)
        cur = data[i]
        st = ds
s += calc_haffman(cur)
print(s)

print(calc_haffman(data.flatten()))

def shift(data):
    result = []
    for line in data:
        row = line.tolist()
        result.append(len(set(row)))
        
        # nums = sorted(list(set(row)))

        count = [0] * 256
        for i in row:
            count[i] += 1
        pr = sorted([(count[i], i) for i in range(256) if count[i]], reverse=True)
        nums = [p for _, p in pr]

        result += nums
        result += list(map(nums.index, line))

    return result

# data = np.array(mp(np.array(data).flatten()))
data = np.array(mp(np.array(data).flatten())).reshape((n, m))
print(np.count_nonzero(data == 0), np.count_nonzero(data == 1), np.count_nonzero(data == 2), np.count_nonzero(data == 3))
data = np.array(shift(data))
print(np.count_nonzero(data == 0), np.count_nonzero(data == 1), np.count_nonzero(data == 2), np.count_nonzero(data == 3))

print(calc_haffman(data))

# exit(0)

# print(calc_haffman(data))

# data = np.array(mp(np.array(data).flatten()))

print(np.mean(data))

for x in data.flatten():
    cnt[x] = cnt.get(x, 0) + 1

x_values = np.array(list(cnt.keys()))
y_values = np.array(list(cnt.values()))

plt.bar(x_values, y_values, width=1, color='b')
plt.grid()
# plt.show()
plt.savefig("plot.png")
