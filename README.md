# Transpilador-Compiladores

## Entrada (Codigo C)
```c
#include <stdio.h>
#include <stdlib.h>

// Define some constants using #define
#define INITIAL_SIZE 0
#define VALUE_COUNT 5
#define VALUE_1 10
#define VALUE_2 20
#define VALUE_3 30
#define VALUE_4 40
#define VALUE_5 50

// Function to add an integer to the list
int* add_to_list(int* list, int* size, int value) {
    (*size)++;
    int* temp = realloc(list, (*size) * sizeof(int));
    if (temp == NULL) {
        printf("Memory allocation failed!\n");
        free(list);
        exit(1);
    }
    list = temp;
    list[(*size) - 1] = value;
    return list;
}

// Function to print the list
void print_list(int* list, int size) {
    printf("Current List (%d items): ", size);
    for (int i = 0; i < size; i++) {
        printf("%d ", list[i]);
    }
    printf("\n");
}

int main() {
    int* list = NULL;
    int size = INITIAL_SIZE;

    // Use defined constants for values to add
    int values_to_add[VALUE_COUNT] = {
        VALUE_1, VALUE_2, VALUE_3, VALUE_4, VALUE_5
    };

    for (int i = 0; i < VALUE_COUNT; i++) {
        list = add_to_list(list, &size, values_to_add[i]);
        print_list(list, size);
    }

    printf("\nFinal list:\n");
    print_list(list, size);

    free(list);
    printf("Memory successfully freed.\n");

    return 0;
}
```

## Transpilacion Fase 1 (Regex)
```cpp
// #include <stdio.h> // Reemplazado por <iostream>
#include <iostream>
#include <string>
#include <array>
#include <stdlib.h>

// Define some constants using #define
constexpr int INITIAL_SIZE = 0; // Convertido de #define
constexpr int VALUE_COUNT = 5; // Convertido de #define
constexpr int VALUE_1 = 10; // Convertido de #define
constexpr int VALUE_2 = 20; // Convertido de #define
constexpr int VALUE_3 = 30; // Convertido de #define
constexpr int VALUE_4 = 40; // Convertido de #define
constexpr int VALUE_5 = 50; // Convertido de #define

// Function to add an integer to the list
int* add_to_list(int* list, int* size, int value) {
    (*size)++;
    int* temp = realloc(list, (*size) * sizeof(int));
    if (temp == nullptr) {
        std::cout << "Memory allocation failed!\n";
        free(list);
        exit(1);
    }
    list = temp;
    list[(*size) - 1] = value;
    return list;
}

// Function to print the list
void print_list(int* list, int size) {
    std::cout << "Current List (" << size << " items): ";
    for (int i = 0; i < size; i++) {
        std::cout << list[i] << " ";
    }
    std::cout << "\n";
}

int main() {
    int* list = nullptr;
    int size = INITIAL_SIZE;

    // Use defined constants for values to add
    int values_to_add[VALUE_COUNT] = {
        VALUE_1, VALUE_2, VALUE_3, VALUE_4, VALUE_5
    };

    for (int i = 0; i < VALUE_COUNT; i++) {
        list = add_to_list(list, &size, values_to_add[i]);
        print_list(list, size);
    }

    std::cout << "\nFinal list:\n";
    print_list(list, size);

    free(list);
    std::cout << "Memory successfully freed.\n";

    return 0;
}
```

## Transpilacion Fase 2 (LLM)
```cpp
#include <iostream>
#include <string>
#include <array>
#include <cstdlib>
#include <memory>
#include <vector>

using namespace std;

constexpr int INITIAL_SIZE = 0;
constexpr int VALUE_COUNT = 5;
constexpr int VALUE_1 = 10;
constexpr int VALUE_2 = 20;
constexpr int VALUE_3 = 30;
constexpr int VALUE_4 = 40;
constexpr int VALUE_5 = 50;

// Function to add an integer to the list
vector<int> add_to_list(vector<int> list, int value) {
    list.push_back(value);
    return list;
}

// Function to print the list
void print_list(vector<int> list) {
    cout << "Current List (" << list.size() << " items): ";
    for (int i = 0; i < list.size(); i++) {
        cout << list[i] << " ";
    }
    cout << "\n";
}

int main() {
    vector<int> list = {};

    // Use defined constants for values to add
    int values_to_add[VALUE_COUNT] = {
        VALUE_1, VALUE_2, VALUE_3, VALUE_4, VALUE_5
    };

    for (int i = 0; i < VALUE_COUNT; i++) {
        list = add_to_list(list, values_to_add[i]);
        print_list(list);
    }

    cout << "\nFinal list:\n";
    print_list(list);

    // No need to free memory

	return 0;
}
```
