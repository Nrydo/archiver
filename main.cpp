#include <bits/stdc++.h>

#include <ext/pb_ds/detail/standard_policies.hpp>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

#define byte uint8_t
#define bit bool

using namespace std;

using namespace __gnu_pbds;
template <typename T> using ordered_set = tree <T, null_type, less < T >, rb_tree_tag, tree_order_statistics_node_update>;

using bytes = vector<byte>;
using bits = vector<bool>;

bits to_bits(const bytes& bytes, bool padding = true) {
    bits result;
    result.reserve(bytes.size() * 8);
    for (byte x : bytes) {
        for (int i = 0; i < 8; i++) {
            result.push_back((x >> i) & 1);
        }
    }
    if (padding) {
        bit t = result.back();
        while (result.back() == t) {
            result.pop_back();
        }
    }
    return result;
}

bytes to_bytes(const bits& bits) {
    bytes result((bits.size() + 7) / 8);
    for (int i = 0; i < bits.size(); i++) {
        result[i / 8] |= bits[i] << (i % 8);
    }
    if (bits.size() % 8) {
        for (int i = bits.size() % 8; i < 8; i++) {
            result.back() |= (bits.back() ^ 1) << i;
        }
    } else {
        result.push_back(255 * !bits.back());
    }
    return result;
}

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

struct RLE : Archiver {
    int block = 4;

    bytes encrypt(const bytes& data) override {
        auto code = to_bits(data);
        bits result;
        int max_count = 1 << (block - 1);
        for (int i = 0; i < code.size(); i++) {
            int count = 1;
            while (i + count < code.size() && code[i + count] == code[i] && count < max_count) {
                count++;
            }
            count--;
            for (int j = 0; j < block - 1; j++) {
                result.push_back((count >> j) & 1);
            }
            result.push_back(code[i]);
            i += count;
        }
        return to_bytes(result);
    }

    bytes decrypt(const bytes& data) override {
        auto code = to_bits(data);
        bits result;
        int i = 0;
        while (i < code.size()) {
            int count = 0;
            for (int j = 0; j < block - 1; j++) {
                count |= (code[i + j] << j);
            }
            i += block - 1;
            for (int j = 0; j <= count; j++) {
                result.push_back(code[i]);
            }
            i++;
        }
        return to_bytes(result);
    }
};

struct BWT : Archiver {
    vector<int> build_suffix_array(const bytes& data) {
        int n = data.size();
        vector<int> sa(n), ranks(n), new_ranks(n);
        vector<int> cnt(max(256, n), 0);
    
        for (int i = 0; i < n; ++i)
            cnt[data[i]]++;
        for (int i = 1; i < 256; ++i)
            cnt[i] += cnt[i - 1];
        for (int i = n - 1; i >= 0; --i)
            sa[--cnt[data[i]]] = i;
    
        ranks[sa[0]] = 0;
        int classes = 1;
        for (int i = 1; i < n; ++i) {
            if (data[sa[i]] != data[sa[i - 1]])
                classes++;
            ranks[sa[i]] = classes - 1;
        }

        for (int k = 0; (1 << k) < n; ++k) {
            for (int i = 0; i < n; ++i)
                sa[i] = (sa[i] - (1 << k) + n) % n;
    
            fill(cnt.begin(), cnt.begin() + classes, 0);
            for (int i = 0; i < n; ++i)
                cnt[ranks[sa[i]]]++;
            for (int i = 1; i < classes; ++i)
                cnt[i] += cnt[i - 1];
            for (int i = n - 1; i >= 0; --i)
                new_ranks[--cnt[ranks[sa[i]]]] = sa[i];
            sa.swap(new_ranks);
    
            new_ranks[sa[0]] = 0;
            classes = 1;
            for (int i = 1; i < n; ++i) {
                int cur1 = ranks[sa[i]], cur2 = ranks[(sa[i] + (1 << k)) % n];
                int prev1 = ranks[sa[i - 1]], prev2 = ranks[(sa[i - 1] + (1 << k)) % n];
                if (cur1 != prev1 || cur2 != prev2)
                    classes++;
                new_ranks[sa[i]] = classes - 1;
            }
            ranks.swap(new_ranks);
        }
    
        return sa;
    }

    bytes encrypt(const bytes& data) override {
        int n = data.size();
        auto suffix_array = build_suffix_array(data);
        bytes result(n + 4);
        for (int i = 0; i < n; i++) {
            result[i] = data[(suffix_array[i] + n - 1) % n];
        }
        int index = find(suffix_array.begin(), suffix_array.end(), 0) - suffix_array.begin();
        result[n] = index & 255;
        result[n + 1] = (index >> 8) & 255;
        result[n + 2] = (index >> 16) & 255;
        result[n + 3] = (index >> 24) & 255;
        return result;
    }

    bytes decrypt(const bytes& data) override {
        auto copy = data;
        int k = 0;
        for (int i = 0; i < 4; i++) {
            k = (k << 8) | copy.back();
            copy.pop_back();
        }
        int n = copy.size();
        vector<int> cnt(256), start(256);
        for (byte x : copy) cnt[x]++;
        
        for (int i = 1; i < 256; i++) start[i] = start[i - 1] + cnt[i - 1];
        
        vector<int> next(n);
        for (int i = 0; i < n; i++) {
            next[start[copy[i]]++] = i;
        }
        
        bytes result(n);
        int idx = next[k];
        for (int i = 0; i < n; i++) {
            result[i] = copy[idx];
            idx = next[idx];
        }
        return result;
    }
};

struct MTF : Archiver {
    bytes encrypt(const bytes& data) override {
        ordered_set < pair < int, int > > st;

        vector < int > marks(256, 0);
        for (int i = 0; i < 256; i++) {
            marks[i] = i;
            st.insert({marks[i], i});
        }

        bytes enc;
        int first = 0;
        for (auto to: data) {
            int cnt = st.order_of_key({marks[to], to});
            enc.push_back(cnt);
            st.erase({marks[to], to});
            marks[to] = --first;
            st.insert({marks[to], to});
        }

        return enc;
    }

    bytes decrypt(const bytes& data) override {
        ordered_set < pair < int, int > > st;
        vector < int > marks(256, 0);
        for (int i = 0; i < 256; i++) {
            marks[i] = i;
            st.insert({marks[i], i});
        }

        bytes dec;
        int first = 0;
        for (auto to: data) {
            pair<int, int> cnt = *st.find_by_order(to);
            dec.push_back(cnt.second);
            st.erase(cnt);
            marks[cnt.second] = --first;
            st.insert({marks[cnt.second], cnt.second});
        }

        return dec;
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
        return to_bytes(code);
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
        auto code = to_bits(data);
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

struct BZIP2 : Archiver {
    bytes encrypt(const bytes &data) override {
        return Huffman().encrypt(MTF().encrypt(BWT().encrypt(data)));
    }

    bytes decrypt(const bytes &data) override {
        return BWT().decrypt(MTF().decrypt(Huffman().decrypt(data)));
    }
};

struct MultiArchiver : Archiver {
    vector<Archiver*> archivers = {
        new Identity(),
        // new RLE(),
        // new Huffman(),
        new BZIP2(),
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
#ifdef LOCAL
    // freopen("input.txt", "r", stdin);
    // freopen("tests/simple.txt", "r", stdin);
    freopen("tests/70", "r", stdin);
    freopen("output.txt", "w", stdout);
#endif
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

        int n, m, nsize;
        if (result[0] & 128) {
            n = result[0] & 127;
            nsize = 1;
        } else {
            n = result[1] + result[0] * 256;
            nsize = 2;
        }
        m = (result.size() - nsize) / n;
        cout << n << ' ' << m << '\n';
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                cout << int16_t(result[nsize + i * m + j]) - 128 << ' ';
            }
            cout << '\n';
        }
    } else {
        int n, m;
        cin >> n >> m;
        bytes data(1 + (n >= 128) + n * m);
        int nsize;
        if (n < 128) {
            data[0] = n | 128;
            nsize = 1;
        } else {
            data[0] = n / 256;
            data[1] = n % 256;
            nsize = 2;
        }
        for (auto& x : data | ranges::views::drop(nsize)) {
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
