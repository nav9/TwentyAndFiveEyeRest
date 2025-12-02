#include "SettingsSearch.hpp"
#include <algorithm>
#include <vector>

int levenshtein(const std::string& a, const std::string& b) {
    const size_t n = a.size();
    const size_t m = b.size();
    if(n == 0) return static_cast<int>(m);
    if(m == 0) return static_cast<int>(n);
    std::vector<int> prev(m+1), cur(m+1);
    for(size_t j = 0; j <= m; ++j) prev[j] = static_cast<int>(j);
    for(size_t i = 1; i <= n; ++i) {
        cur[0] = static_cast<int>(i);
        for(size_t j = 1; j <= m; ++j) {
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            cur[j] = std::min({ prev[j] + 1, cur[j-1] + 1, prev[j-1] + cost });
        }
        prev.swap(cur);
    }
    return prev[m];
}
