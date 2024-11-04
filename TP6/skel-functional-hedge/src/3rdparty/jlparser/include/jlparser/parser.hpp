#ifndef PARSER_H
#define PARSER_H

#include "pnl/pnl_matrix.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <type_traits>
#include <cctype>
#include <variant>
#include "jlparser/json_helper.hpp"

#define MAX_CHAR_LINE 1024
// #define DEBUG

/* list of possible types */
typedef enum
{
    T_NULL,
    T_INT,
    T_LONG,
    T_DOUBLE,
    T_VECTOR,
    T_STRING,
    T_PTR
} T_type;

class TypeVal
{
  public:
    T_type type;
    std::variant<int, unsigned long, double, std::vector<double>, std::string, void*> Val;
    TypeVal();
    TypeVal(const TypeVal&);
    // Be sure not to delete anything because we rely on copy by address.
    ~TypeVal();
    TypeVal& operator=(const TypeVal& v);
    void print(const std::string& s) const;
};

struct comp
{
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
        std::string::const_iterator first1 = lhs.begin();
        std::string::const_iterator first2 = rhs.begin();
        std::string::const_iterator last1 = lhs.end();
        std::string::const_iterator last2 = rhs.end();
        for (; (first1 != last1) && (first2 != last2); ++first1, (void)++first2) {
            char a = std::tolower(*first1);
            char b = std::tolower(*first2);
            if (a < b) return true;
            if (b < a) return false;
        }
        return (first1 == last1) && (first2 != last2);
    }
};

typedef std::map<std::string, TypeVal, comp> Hash;

class TxtParser
{
  public:
    Hash M;
    TxtParser() {}
    TxtParser(const TxtParser&);
    TxtParser(const char* inputFile);
    ~TxtParser();
    TxtParser& operator=(const TxtParser& P);

    template<typename T>
    bool extract(const std::string& key, T& out, bool go_on = false) const
    {
        static_assert(!std::is_same<PnlVect*, T>(), "Use the specialized version.");
        Hash::const_iterator it;
        if (check_if_key(it, key) == false) {
            if (!go_on) {
                std::cout << "Key " << key << " not found." << std::endl;
                abort();
            } else
                return false;
        }
        try {
            out = std::get<T>(it->second.Val);
            return true;
        } catch (std::bad_variant_access e) {
            std::cout << "bad get for " << key << std::endl;
            abort();
        }
    }

    bool extract(const std::string& key, PnlVect*& out, int size, bool go_on = false) const;

    template<typename T>
    bool set(const std::string& key, const T& in)
    {
        Hash::iterator it;
        if ((it = M.find(key)) == M.end()) return false;
        try {
            std::get<T>(it->second.Val) = in;
            return true;
        } catch (std::bad_variant_access e) {
            std::cout << "bad get for " << key << std::endl;
            abort();
        }
    }

    /**
     * Insert a new pair in the map or set M[key] to the new value if the key already exists in the map
     *
     * @tparam T the template type of the element to be inserted
     * @param key the key
     * @param t the type of the elements as an integer
     * @param in the element itself
     */
    template<typename T>
    void insert(const std::string& key, const T_type& t, const T& in)
    {
        if (M.find(key) != M.end()) {
            set<T>(key, in);
            return;
        }
        TypeVal V;
        V.type = t;
        V.Val = in;
        M[key] = V;
    }

    void print() const
    {
        Hash::const_iterator it;
        for (it = M.begin(); it != M.end(); it++) it->second.print(it->first);
    }

  private:
    bool check_if_key(Hash::const_iterator& it, const std::string& key) const;
    void ReadInputFile(const char* inputFile);
    void add(char RedLine[]);
    char type_ldelim = '<';
    char type_rdelim = '>';
};


class JsonParser
{
  public:
    nlohmann::json J;
    JsonParser() {}
    JsonParser(const char* inputFile);

    template<typename T>
    bool extract(const std::string& key, T& out, bool go_on = false) const
    {
        static_assert(!std::is_same<PnlVect*, T>(), "Use the specialized version.");
        if (! J.contains(key)) {
            if (!go_on) {
                std::cout << "Key " << key << " not found." << std::endl;
                abort();
            } else
                return false;
        }
        J.at(key).get_to(out);
        return true;

    }

    bool extract(const std::string& key, PnlVect*& out, int size, bool go_on = false) const;

    template<typename T>
    bool set(const std::string& key, const T& in)
    {
        if (!J.contains(key)) return false;
        J.at(key) = in;
        return true;
    }

    /**
     * Insert a new pair in the map or set M[key] to the new value if the key already exists in the map
     *
     * @tparam T the template type of the element to be inserted
     * @param key the key
     * @param t the type of the elements as an integer
     * @param in the element itself
     */
    template<typename T>
    void insert(const std::string& key, const T_type& t, const T& in)
    {
        J.at(key) = in;
    }

    void print() const
    {
        std::cout << std::setw(4)<< J << std::endl;
    }
};

template<typename...Func>
struct overloaded : Func... {
    using Func::operator()...;
};

template<typename...Func> overloaded(Func...) -> overloaded<Func...>;

class IParser
{
  public:
    IParser(const char* inputFile);

    template<typename T>
    void insert(const std::string& key, const T_type& t, const T& in) {
        switch(actualParser.index()) {
            case 0: std::get<0>(actualParser).insert(key, t, in); break;
            case 1: std::get<1>(actualParser).insert(key, t, in); break;
            default: abort();
        }
    }

    virtual bool extract(const std::string& key, PnlVect*& out, int size, bool go_on = false) const {
        switch(actualParser.index()) {
            case 0: return std::get<0>(actualParser).extract(key, out, size, go_on); break;
            case 1: return std::get<1>(actualParser).extract(key, out, size, go_on); break;
            default: abort();
        }
    }

    template<typename T>
    bool extract(const std::string& key, T& out, bool go_on = false) const {
        switch(actualParser.index()) {
            case 0: return std::get<0>(actualParser).extract(key, out, go_on); break;
            case 1: return std::get<1>(actualParser).extract(key, out, go_on); break;
            default: abort();
        }
    }

    template<typename T>
    bool set(const std::string& key, const T& in) {
        switch(actualParser.index()) {
            case 0: return std::get<0>(actualParser).set(key, in); break;
            case 1: return std::get<1>(actualParser).set(key, in); break;
            default: abort();
        }
    }

    void print() const;

private:
    std::variant<TxtParser, JsonParser> actualParser;
};

#endif
