#ifndef SRC_HPP
#define SRC_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

class BasicException {
protected:
    std::string msg_str;
    const char *message;

public:
    explicit BasicException(const char *_message) : msg_str(_message) {
        message = msg_str.c_str();
    }

    virtual const char *what() const {
        return msg_str.c_str();
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char* _message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char* _message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    char types[80];
};

class Pokedex {
private:
    std::map<int, Pokemon> pokes;
    std::string filename;

    bool isValidType(const std::string& type) const {
        static const std::set<std::string> valid_types = {
            "fire", "water", "grass", "electric", "flying", "ground", "dragon"
        };
        return valid_types.count(type) > 0;
    }

    std::vector<std::string> splitTypes(const std::string& types_str) const {
        std::vector<std::string> res;
        std::stringstream ss(types_str);
        std::string item;
        while (std::getline(ss, item, '#')) {
            res.push_back(item);
        }
        return res;
    }

    void checkName(const char* name) const {
        if (!name || name[0] == '\0') {
            throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name ? name : "") + ")").c_str());
        }
        for (int i = 0; name[i] != '\0'; ++i) {
            if (!isalpha(name[i])) {
                throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
            }
        }
    }

    void checkId(int id) const {
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
    }

    void checkTypes(const char* types) const {
        std::string t_str(types);
        std::stringstream ss(t_str);
        std::string item;
        while (std::getline(ss, item, '#')) {
            if (!isValidType(item)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + item + ")").c_str());
            }
        }
    }

    float getMultiplier(const std::string& attacker, const std::string& defender) const {
        static std::map<std::string, std::map<std::string, float>> table = {
            {"fire", {{"fire", 0.5}, {"water", 0.5}, {"grass", 2}, {"electric", 1}, {"flying", 1}, {"ground", 1}, {"dragon", 0.5}}},
            {"water", {{"fire", 2}, {"water", 0.5}, {"grass", 0.5}, {"electric", 1}, {"flying", 1}, {"ground", 2}, {"dragon", 0.5}}},
            {"grass", {{"fire", 0.5}, {"water", 2}, {"grass", 0.5}, {"electric", 1}, {"flying", 0.5}, {"ground", 2}, {"dragon", 0.5}}},
            {"electric", {{"fire", 1}, {"water", 2}, {"grass", 0.5}, {"electric", 1}, {"flying", 2}, {"ground", 0}, {"dragon", 0.5}}},
            {"flying", {{"fire", 1}, {"water", 1}, {"grass", 2}, {"electric", 0.5}, {"flying", 1}, {"ground", 1}, {"dragon", 1}}},
            {"ground", {{"fire", 2}, {"water", 1}, {"grass", 0.5}, {"electric", 2}, {"flying", 0}, {"ground", 1}, {"dragon", 1}}},
            {"dragon", {{"fire", 1}, {"water", 1}, {"grass", 1}, {"electric", 1}, {"flying", 1}, {"ground", 1}, {"dragon", 2}}}
        };
        return table[attacker][defender];
    }

public:
    explicit Pokedex(const char *_fileName) : filename(_fileName) {
        std::ifstream in(filename);
        if (in) {
            int count;
            if (in >> count) {
                for (int i = 0; i < count; ++i) {
                    Pokemon p;
                    in >> p.name >> p.id >> p.types;
                    pokes[p.id] = p;
                }
            }
        }
    }

    ~Pokedex() {
        std::ofstream out(filename);
        out << pokes.size() << "\n";
        for (auto const& pair : pokes) {
            out << pair.second.name << " " << pair.second.id << " " << pair.second.types << "\n";
        }
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        checkName(name);
        checkId(id);
        checkTypes(types);
        if (pokes.count(id)) return false;
        Pokemon p;
        strncpy(p.name, name, 11);
        p.name[11] = '\0';
        p.id = id;
        strncpy(p.types, types, 79);
        p.types[79] = '\0';
        pokes[id] = p;
        return true;
    }

    bool pokeDel(int id) {
        checkId(id);
        if (!pokes.count(id)) return false;
        pokes.erase(id);
        return true;
    }

    std::string pokeFind(int id) const {
        checkId(id);
        auto it = pokes.find(id);
        if (it != pokes.end()) {
            return it->second.name;
        }
        return "None";
    }

    std::string typeFind(const char *types) const {
        checkTypes(types);
        std::vector<std::string> query_types = splitTypes(types);
        std::vector<std::string> found_names;
        for (auto const& pair : pokes) {
            std::vector<std::string> p_types = splitTypes(pair.second.types);
            bool has_all = true;
            for (const auto& qt : query_types) {
                if (std::find(p_types.begin(), p_types.end(), qt) == p_types.end()) {
                    has_all = false;
                    break;
                }
            }
            if (has_all) {
                found_names.push_back(pair.second.name);
            }
        }
        if (found_names.empty()) return "None";
        std::string res = std::to_string(found_names.size()) + "\n";
        for (size_t i = 0; i < found_names.size(); ++i) {
            res += found_names[i];
            if (i + 1 < found_names.size()) res += "\n";
        }
        return res;
    }

    float attack(const char *type, int id) const {
        checkTypes(type);
        checkId(id);
        auto it = pokes.find(id);
        if (it == pokes.end()) return -1.0f;
        
        std::vector<std::string> p_types = splitTypes(it->second.types);
        float multiplier = 1.0f;
        for (const auto& pt : p_types) {
            multiplier *= getMultiplier(type, pt);
        }
        return multiplier;
    }

    int catchTry() const {
        if (pokes.empty()) return 0;
        
        std::set<int> owned;
        owned.insert(pokes.begin()->first);
        
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto const& pair : pokes) {
                int target_id = pair.first;
                if (owned.count(target_id)) continue;
                
                const Pokemon& target_p = pair.second;
                bool can_catch = false;
                for (int owned_id : owned) {
                    const Pokemon& owned_p = pokes.at(owned_id);
                    std::vector<std::string> owned_types = splitTypes(owned_p.types);
                    
                    for (const auto& ot : owned_types) {
                        float mult = 1.0f;
                        std::vector<std::string> target_types = splitTypes(target_p.types);
                        for (const auto& tt : target_types) {
                            mult *= getMultiplier(ot, tt);
                        }
                        if (mult >= 2.0f) {
                            can_catch = true;
                            break;
                        }
                    }
                    if (can_catch) break;
                }
                
                if (can_catch) {
                    owned.insert(target_id);
                    changed = true;
                }
            }
        }
        return owned.size();
    }

    struct iterator {
        std::map<int, Pokemon>::iterator it;
        std::map<int, Pokemon>* map_ptr;

        iterator() : map_ptr(nullptr) {}
        iterator(std::map<int, Pokemon>::iterator _it, std::map<int, Pokemon>* _map_ptr) 
            : it(_it), map_ptr(_map_ptr) {}

        iterator &operator++() {
            if (it == map_ptr->end()) {
                throw IteratorException("Iterator Error: Invalid Iterator");
            }
            ++it;
            return *this;
        }
        iterator &operator--() {
            if (it == map_ptr->begin()) {
                throw IteratorException("Iterator Error: Invalid Iterator");
            }
            --it;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        iterator & operator = (const iterator &rhs) {
            it = rhs.it;
            map_ptr = rhs.map_ptr;
            return *this;
        }
        bool operator == (const iterator &rhs) const {
            return it == rhs.it;
        }
        bool operator != (const iterator &rhs) const {
            return it != rhs.it;
        }
        Pokemon & operator*() const {
            if (it == map_ptr->end()) {
                throw IteratorException("Iterator Error: Dereference Error");
            }
            return it->second;
        }
        Pokemon *operator->() const {
            if (it == map_ptr->end()) {
                throw IteratorException("Iterator Error: Dereference Error");
            }
            return &(it->second);
        }
    };

    iterator begin() {
        return iterator(pokes.begin(), &pokes);
    }

    iterator end() {
        return iterator(pokes.end(), &pokes);
    }
};

#endif
