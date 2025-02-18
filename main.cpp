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

struct Huffman : Archiver {
    struct Node {
        Node* l;
        Node* r;
        int freq;
        byte value;

        ~Node() {
            delete l;
            delete r;
        }
    };

    Node* build_tree(const bytes& data) {
        vector<int> counts(256);
        for (auto x : data) {
            counts[x]++;
        }
        vector<Node*> nodes;
        for (int i = 0; i < 256; i++) {
            if (counts[i] > 0) {
                nodes.push_back(new Node{nullptr, nullptr, counts[i], byte(i)});
            }
        }
        sort(nodes.begin(), nodes.end(), [](auto a, auto b) { return a->freq < b->freq; });
        queue<Node*>* c, a, b;
        for (auto x : nodes) {
            a.push(x);
        }
        Node* v;
        Node* u;
        while (a.size() + b.size() > 1) {
            v = (c = &(b.empty() || (!a.empty() && a.front()->freq < b.front()->freq) ? a : b))->front();
            c->pop();
            u = (c = &(b.empty() || (!a.empty() && a.front()->freq < b.front()->freq) ? a : b))->front();
            c->pop();
            b.push(new Node(v, u, v->freq + u->freq, 0));
        }
        return a.empty() ? b.front() : a.front();
    }

    void build_biection(Node* node, vector<bool>& code, vector<vector<bool>>& biection) {
        if (node->l) {
            code.push_back(0);
            build_biection(node->l, code, biection);
            code.back() = 1;
            build_biection(node->r, code, biection);
            code.pop_back();
        } else {
            biection[node->value] = code;
        }
    }

    void build_biection(Node* node, vector<vector<bool>>& biection) {
        vector<bool> code;
        build_biection(node, code, biection);
    }

    void encrypt_tree(Node* node, vector<bool>& code) {
        if (node->l) {
            code.push_back(1);
            encrypt_tree(node->l, code);
            encrypt_tree(node->r, code);
        } else {
            code.push_back(0);
            for (int i = 0; i < 8; i++) {
                code.push_back((node->value >> i) & 1);
            }
        }
    }

    bytes encrypt(const bytes& data) override {
        Node* root = build_tree(data);
        vector<bool> code;
        vector<vector<bool>> biection(256);
        encrypt_tree(root, code);
        build_biection(root, biection);
        delete root;
        for (byte value : data) {
            code.insert(code.end(), biection[value].begin(), biection[value].end());
        }
        bytes result((code.size() + 7) / 8);
        for (int i = 0; i < code.size(); i++) {
            result[i / 8] |= code[i] << (i % 8);
        }
        if (code.size() % 8) {
            for (int i = code.size() % 8; i < 8; i++) {
                result.back() |= (code.back() ^ 1) << i;
            }
        } else {
            result.push_back(255 * !code.back());
        }
        return result;
    }

    Node* get_tree(const vector<bool>& code, int& cur) {
        Node* root = new Node(nullptr, nullptr, 0, 0);
        if (code[cur++]) {
            root->l = get_tree(code, cur);
            root->r = get_tree(code, cur);
        } else {
            for (int i = 0; i < 8; i++) {
                root->value |= code[cur++] << i;
            }
        }
        return root;
    }

    bytes decrypt(const bytes& data) override {
        vector<bool> code;
        for (byte x : data) {
            for (int i = 0; i < 8; i++) {
                code.push_back((x >> i) & 1);
            }
        }
        bool t = code.back();
        while (code.back() == t) {
            code.pop_back();
        }
        int cur = 0;
        Node* root = get_tree(code, cur);
        Node* v = root;
        bytes result;
        for (; cur < code.size(); cur++) {
            if (code[cur]) {
                v = v->r;
            } else {
                v = v->l;
            }
            if (v->l == nullptr) {
                result.push_back(v->value);
                v = root;
            }
        }
        delete root;
        return result;
    }
};

struct MultiArchiver : Archiver {
    vector<Archiver*> archivers = {
        // new Identity(),
        new Huffman()
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

struct BlockArchiver : Archiver {
    MultiArchiver archiver;
    const int block_size = 8;

    bytes encrypt(const bytes& data) override {
        bytes buffer, result;
        for (auto x : data) {
            buffer.push_back(x);
            if (buffer.size() == block_size) {
                buffer = archiver.encrypt(buffer);
                result.push_back(buffer.size());
                if (result.size() == 1)
                    cout << buffer.size() << '\n';
                result.insert(result.end(), buffer.begin(), buffer.end());
                buffer.clear();
            }
        }
        if (!buffer.empty()) {
            buffer = archiver.encrypt(buffer);
            result.push_back(buffer.size());
            cout << "buffer: " << buffer.size() << '\n';
            result.insert(result.end(), buffer.begin(), buffer.end());
            buffer.clear();
        }
        return archiver.encrypt(data);
    }

    bytes decrypt(const bytes& data) override {
        bytes buffer, result;
        int count = 0;
        for (auto x : data) {
            if (count == 0) {
                count = x;
                cout << "count: " << count << '\n';
                continue;
            }
            count--;
            buffer.push_back(x);
            if (count == 0) {
                buffer = archiver.decrypt(buffer);
                result.insert(result.end(), buffer.begin(), buffer.end());
                buffer.clear();
            }
        }
        return result;
    }
};

int32_t main() {
    // freopen("tests/275", "r", stdin);
    freopen("tests/70", "r", stdin);
    // freopen("tests/small.txt", "r", stdin);
    // freopen("tests/simple.txt", "r", stdin);
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

        BlockArchiver archiver;
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
        
        // BlockArchiver archiver;
        MultiArchiver archiver;
        auto result = archiver.encrypt(data);
        
        cout << result.size() << '\n';
        // for (auto x : result) {
        //     cout << uint16_t(x) << ' ';
        // }
        // cout << '\n';

        auto ans = archiver.decrypt(result);
        if (ans == data) {
            cout << "OK\n";
        } else {
            cout << "FAIL\n";
        }
    }
    return 0;
}
