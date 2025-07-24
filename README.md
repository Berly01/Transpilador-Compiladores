# Transpilador-Compiladores

Programa en C que implementa una lista dinámica de enteros usando:

* Macros #define para constantes
* Manejo manual de memoria con malloc/realloc/free
* Punteros y aritmética de punteros
* I/O con printf de la biblioteca estándar de C
* Gestión explícita del tamaño de la lista mediante punteros


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

El transpilador basado en reglas realiza conversiones sintácticas directas:

* #include <stdio.h> a #include <iostream>
* #define a constexpr int
* printf a std::cout
* NULL a nullptr
* Mantiene la lógica de C: sigue usando realloc(), free(), punteros y manejo manual de memoria

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

El modelo realiza una transformación semántica profunda:

* Cambio de paradigma: De punteros a contenedores STL (vector<int>)
* Eliminación de memoria manual: Reemplaza realloc/free con gestión automática
* Simplificación de funciones: add_to_list() usa push_back() nativo
* Modernización: Agrega using namespace std y headers C++ apropiados
* Funcionalidad preservada: El comportamiento del programa permanece idéntico

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
