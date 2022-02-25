#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <memory>


//==  DEFINITION  ==============================================================

typedef std::vector<std::shared_ptr<int>> test_t;
test_t test;

template <typename T>
class Dict {
    // Structure of Dict cell
    typedef struct {
        u_int64_t hash = 0;
        std::string key{};
    	T value{};
    } DictCell;
    static const u_int32_t default_size = 8;

    u_int32_t length;  // How many cells occupied
    u_int32_t size;    // How many cells at all
    DictCell *table;   // Array of cells aka Table

    // Get hashnumber from string key
    u_int64_t get_hash(std::string &key);
    // Create new table and move data to it
    void change_size(u_int32_t new_size);
    // Copy all pairs from current table to other one
    void copy_in_table(DictCell *dest_table, u_int32_t &dest_size);

public:
    Dict();
    explicit Dict(u_int32_t start_size);
    virtual ~Dict();

    // Add or update key-value pair
    void insert(std::string &key, T &value);
    // Get value by key, default of T if not found
    T get(std::string &key);
    // Delete key-value pair from table
    void remove(std::string &key);
    
    void memview(); // print method
};


//==  DECLARATION  =============================================================

template <typename T>
Dict<T>::Dict() :
        length(0),
        size(default_size),
        table(new DictCell[default_size])
{}


template <typename T>
Dict<T>::Dict(u_int32_t start_size) :
        length(0),
        size(start_size),
        table(new DictCell[start_size])
{}


template <typename T>
Dict<T>::~Dict() {
    delete[](table);
}


// I don't know how I came to this, but this is my own hash function for strings
template<typename T>
u_int64_t Dict<T>::get_hash(std::string &key) {
    u_int64_t hash = 17;
    char ch;
    for (auto &ch : key) {
        hash *= static_cast<u_int64_t>(ch << 1) + 1;  // Avoid multipling by zero
        hash = (hash<<7) | (hash>>( (-7)&63 ));  // Rotary shift to left by 7
        hash ^= ch;
    }
    return hash;
}


template <typename T>
void Dict<T>::change_size(unsigned int new_size) {
    auto new_table = new DictCell[new_size];
    copy_in_table(new_table, new_size);
    delete[](table);
    table = new_table;
    size = new_size;
}


template <typename T>
void Dict<T>::copy_in_table(DictCell *dest_table, u_int32_t &dest_size) {
    DictCell *cell, *new_cell;
    u_int32_t new_index;
    for (u_int32_t old_index = 0; old_index < size; old_index++) {
        // Skip cells without value
        if (table[old_index].key.length() == 0)
            continue;
        
        cell = &table[old_index];
        new_index = cell->hash % dest_size;

        // Find first unoccupied cell in new table
        while (dest_table[new_index].key.length())
            ++new_index %= dest_size;

        new_cell = &dest_table[new_index];
        // Copy data from old cell to new one
        new_cell->hash = cell->hash;
        new_cell->key = cell->key;
        memcpy(&new_cell->value, &cell->value, sizeof(T));
    }
}


template <typename T>
void Dict<T>::insert(std::string &key, T &value) {
    // Increase size if using 3/4 of capacity
    if (length >= 3*size / 4) {
        change_size(size*2);
    }

    u_int64_t hash = get_hash(key);
    u_int32_t index = hash % size;
    // Linear lookup starting from "hash index"
    while (true) {
        if (table[index].hash == hash  // Update value of cell with same hash
            || table[index].key.length() == 0)  // Or write in unoccupied cell
        {
            table[index].hash = hash;
            table[index].key = key;
            memcpy(&table[index].value, &value, sizeof(T));
            break;
        }
        ++index %= size;
    }
    length++;
}


template <typename T>
T Dict<T>::get(std::string &key) {
    u_int64_t hash = get_hash(key);
    u_int32_t index = hash % size;

    // Desired key can't locate after empty cell
    while (table[index].hash != 0) {  
        if (table[index].hash == hash) {
            // If pair was removed return default
            if (table[index].key.length() == 0) break;
            return table[index].value;
        }
        ++index %= size;
    }
    // Return default value of T if no value found
    return T{};
}


template<typename T>
void Dict<T>::remove(std::string &key) {
    u_int64_t hash = get_hash(key);
    u_int32_t index = hash % size;

    // Desired key can't locate after empty cell
    while (table[index].hash != 0) {
        if (table[index].hash == hash) {
            // Leave hash because of lookup algorithm
            table[index].key = {};
            table[index].value = {};
            break;
        }
        ++index %= size;
    }
    length--;

    // Decrease size if using 1/8 of capacity
    if (length <= size / 8 && size > default_size) {
        change_size(size/2);
    }
}


// "Debug" method to print contant of hashtable
template<typename T>
void Dict<T>::memview() {
    for (u_int32_t i = 0; i < size; i++) {
        std::cout
            << std::setw(20) << table[i].hash << " "
            << std::setw(12) << table[i].key << " : ";

        if(std::is_pointer<T>::value) {
            if (table[i].value == nullptr) {
                std::cout << "NULL" << "\n";
            } else {
                std::cout << *table[i].value << "\n";
            }
        } else {
            std::cout << table[i].value << "\n";
        }
    }
    std::cout << std::endl;
}


//==============================================================================
//==  TESTING  =================================================================
//==============================================================================

// Some "value" example class
class Color {
    u_int8_t r = 0;
    u_int8_t g = 0;
    u_int8_t b = 0;
public:
    Color() = default;
    Color(u_int8_t red, u_int8_t green, u_int8_t blue): r(red), g(green), b(blue) {}
    friend std::ostream& operator<<(std::ostream& os, const Color& color);
};


std::ostream &operator<<(std::ostream &os, const Color &color) {
    return os << "<"
        << std::setw(3) << +color.r << ", "
        << std::setw(3) << +color.g << ", "
        << std::setw(3) << +color.b << ">";
}


int main() {
    // Some variables for using as keys
    std::string key1("cat");
    std::string key2("dog");
    std::string key3("penguin");
    std::string key4("coyote");
    std::string key5("tiger");
    std::string key6("lion");
    std::string key7("dolphin");

    // Some variables for using as values (I take random colors)
    auto value1 = new Color{132, 211,  33};
    auto value2 = new Color{121,  11,  56};
    auto value3 = new Color{132,  53,  78};
    auto value4 = new Color{ 55, 116,  65};
    auto value5 = new Color{ 98, 210, 216};
    auto value6 = new Color{ 75, 198, 166};
    auto value7 = new Color{ 46,  15, 255};

    Dict<Color*> dict;

    std::cout << "Hashtable is empty:" << std::endl;
    dict.memview();

    dict.insert(key1, value1);
    dict.insert(key2, value2);
    dict.insert(key3, value3);
    dict.insert(key4, value4);
    dict.insert(key5, value5);
    dict.insert(key6, value6);

    std::cout << "Inserted 6 key-value pairs" << std::endl;
    dict.memview();

    dict.insert(key7, value7);

    std::cout << "After next insert hashtable increases it's size" << std::endl;
    dict.memview();

    std::cout << "Let's get value by key" << std::endl;
    std::cout << key5 << " : " << *dict.get(key5) << "\n" << std::endl;

    dict.remove(key1);
    dict.remove(key3);
    dict.remove(key4);
    dict.remove(key6);
    dict.remove(key7);

    std::cout << "If remove enough pairs then table decreases size" << std::endl;
    dict.memview();

    dict.insert(key6, value6);
    dict.remove(key5);

    std::cout << "Hash still in cell because of lookup algorithm" << std::endl;
    dict.memview();

    std::cout << "It's just works" << std::endl;
    std::cout << key6 << " : " << *dict.get(key6) << "\n" << std::endl;

    return 0;
}
