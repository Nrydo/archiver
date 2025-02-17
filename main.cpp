#include <bits/stdc++.h>

#define byte uint8_t

using namespace std;

using bytes = vector<byte>;

struct Archiver {
    virtual bytes encrypt(const bytes& data) = 0;
    virtual bytes decrypt(const bytes& data) = 0;
    virtual ~Archiver() = default;
};

struct Identity : Archiver {
    bytes encrypt(const bytes& data) override {
        return data;
    }

    bytes decrypt(const bytes& data) override {
        return data;
    }
};

struct MultiArchiver : Archiver {
    vector<Archiver*> archivers = {
        new Identity()
    };

    ~MultiArchiver() {
        for (Archiver* archiver : archivers) {
            delete archiver;
        }
    }

    bytes encrypt(const bytes& data) override {
        bytes best_result;
        for (int i = 0; i < archivers.size(); i++) {
            Archiver* archiver = archivers[i];
            auto result = archiver->encrypt(data);
            result.push_back(i);
            if (best_result.empty() || best_result.size() > result.size()) {
                best_result = std::move(result);
            }
        }
        return best_result;
    }

    bytes decrypt(const bytes& data) override {
        auto archiver = get_archive(data);
        auto temp = data;
        temp.pop_back();
        return archiver->decrypt(temp);
    }

    Archiver* get_archive(const bytes& data) {
        return archivers[data.back()];
    }
};

int32_t main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    bool mode;
    cin >> mode;
    if (mode) {
        int k;
        cin >> k;
        bytes data(k);
        for (auto& x : data) {
            int t;
            cin >> t;
            x = t;
        }

        MultiArchiver archiver;
        auto result = archiver.decrypt(data);

        int n = result[0] + result[1] * 256, m = result[2] + result[3] * 256;
        cout << n << ' ' << m << '\n';
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                cout << int16_t(result[4 + i * m + j]) - 128 << ' ';
            }
            cout << '\n';
        }
    } else {
        int n, m;
        cin >> n >> m;
        bytes data(4 + n * m);
        data[0] = n % 256;
        data[1] = n / 256;
        data[2] = m % 256;
        data[3] = m / 256;
        for (auto& x : data | ranges::views::drop(4)) {
            int t;
            cin >> t;
            x = t + 128;
        }
        
        MultiArchiver archiver;
        auto result = archiver.encrypt(data);
        
        cout << result.size() << '\n';
        for (auto x : result) {
            cout << uint16_t(x) << ' ';
        }
        cout << '\n';
    }
    return 0;
}
