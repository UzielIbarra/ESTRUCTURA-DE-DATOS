#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Definiciones
#define MAX_SIZE 1000
#define MAX_USERS 8
#define MAX_USER_LENGTH 50
#define USERS_FILE "usuarios.txt"
#define PASS_FILE "passwords.txt"
#define QUESTIONS_FILE "preguntas.txt"
#define ANSWERS_FILE "respuestas.txt"
#define MATRIX_SIZE 7
#define MAX_INPUT 1024
#define MAX_STACK 100
#define MAX_QUEUE 100

// Estructuras básicas
struct Metricas {
    int iteraciones;
    int consultas;
    int movimientos;
    double tiempo;
};

struct NodoAVL {
    int dato;
    struct NodoAVL* izquierda;
    struct NodoAVL* derecha;
    int altura;
};

struct Usuario {
    int id;
    char username[MAX_USER_LENGTH];
    char password[MAX_USER_LENGTH];
    char pregunta[100];
    char respuesta[100];
    bool activo;
};

struct DiccionarioDatos {
    int id;
    int fila;
    int columna;
    char valor[MAX_USER_LENGTH];
};

struct Stack {
    int items[MAX_STACK];
    int top;
};

struct Queue {
    int items[MAX_QUEUE];
    int front;
    int rear;
};

// Variables globales
extern struct Usuario usuarios[MAX_USERS];
extern struct DiccionarioDatos diccionario[MAX_USERS];
extern char matrizUsuarios[MATRIX_SIZE][MATRIX_SIZE];
extern char matrizPasswords[MATRIX_SIZE][MATRIX_SIZE];
extern int numUsuarios;

// Prototipos de funciones
bool leerEntero(int* numero);
bool leerString(char* str, int maxLen);
void limpiarBuffer(void);
bool validarUsuario(const char* username);
bool validarPassword(const char* password);
void mostrarMenu(void);
void mostrarMenuPrincipal(void);
void mostrarMetricas(struct Metricas m);
void actualizarDiccionario(void);
void mostrarDiccionario(void);
void cocktailSort(int arr[], int n, struct Metricas* m);
void guardarUsuarios(void);
void combSort(int arr[], int n, struct Metricas* m);

// Prototipos de funciones AVL
struct NodoAVL* newNodo(int dato);
int altura(struct NodoAVL* N);
int maximo(int a, int b);
int getBalance(struct NodoAVL* N);
struct NodoAVL* rotarDerecha(struct NodoAVL* y);
struct NodoAVL* rotarIzquierda(struct NodoAVL* x);
struct NodoAVL* insertar(struct NodoAVL* nodo, int dato, struct Metricas* m);
void inOrder(struct NodoAVL* raiz);
void liberarArbol(struct NodoAVL* raiz);
void avlSort(int arr[], int n, struct Metricas* m);
void imprimirArbol(struct NodoAVL* raiz, int espacio);
void mostrarArbolAVL(struct NodoAVL* raiz);
void imprimirNivel(struct NodoAVL* raiz, int nivel, int espacio);
int obtenerAltura(struct NodoAVL* raiz);
void imprimirArbolMejorado(struct NodoAVL* raiz, int nivel, const char* prefijo, bool esIzquierdo);

// Añadir estos prototipos al inicio del archivo, junto con los otros
void mostrarMenuEstructuras(void);
void initStack(struct Stack* s);
void initQueue(struct Queue* q);
bool isStackEmpty(struct Stack* s);
bool isStackFull(struct Stack* s);
bool isQueueEmpty(struct Queue* q);
bool isQueueFull(struct Queue* q);
void push(struct Stack* s, int value, struct Metricas* m);
int pop(struct Stack* s, struct Metricas* m);
void enqueue(struct Queue* q, int value, struct Metricas* m);
int dequeue(struct Queue* q, struct Metricas* m);

// Matrices para usuarios y contraseñas
char matrizUsuarios[MATRIX_SIZE][MATRIX_SIZE];
char matrizPasswords[MATRIX_SIZE][MATRIX_SIZE];

// ==================== Variables Globales ====================
struct Usuario usuarios[MAX_USERS];
struct DiccionarioDatos diccionario[MAX_USERS];  // <- esta línea es la que faltaba
int numUsuarios = 0;

// ==================== Funciones de Usuarios ====================
void inicializarUsuarios() {
    usuarios[0].id = 1;
    strncpy(usuarios[0].username, "admin", MAX_USER_LENGTH - 1);
    strncpy(usuarios[0].password, "admin", MAX_USER_LENGTH - 1);
    usuarios[0].username[MAX_USER_LENGTH - 1] = '\0';
    usuarios[0].password[MAX_USER_LENGTH - 1] = '\0';
    strcpy(usuarios[0].pregunta, "¿Cuál es la contraseña por defecto?");
    strcpy(usuarios[0].respuesta, "admin");
    usuarios[0].activo = true;
    numUsuarios = 1;
    guardarUsuarios();  // Asegúrate de guardar los datos iniciales
    actualizarDiccionario();
}

void guardarUsuarios() {
    FILE *fUsers = fopen(USERS_FILE, "w");
    FILE *fPass = fopen(PASS_FILE, "w");
    FILE *fQuestions = fopen(QUESTIONS_FILE, "w");
    FILE *fAnswers = fopen(ANSWERS_FILE, "w");

    if (!fUsers || !fPass || !fQuestions || !fAnswers) {
        printf("Error al abrir archivos para guardar.\n");
        return;
    }

    // Guardar solo los nombres de usuario en usuarios.txt
    for (int i = 0; i < numUsuarios; i++) {
        fprintf(fUsers, "%s\n", usuarios[i].username); // Solo el nombre de usuario
        fprintf(fPass, "%d:%s\n", usuarios[i].id, usuarios[i].password);
        fprintf(fQuestions, "%d:%s\n", usuarios[i].id, usuarios[i].pregunta);
        fprintf(fAnswers, "%d:%s\n", usuarios[i].id, usuarios[i].respuesta);
    }

    fclose(fUsers);
    fclose(fPass);
    fclose(fQuestions);
    fclose(fAnswers);
}

void cargarUsuarios() {
    FILE *fUsers = fopen(USERS_FILE, "r");
    FILE *fPass = fopen(PASS_FILE, "r");
    FILE *fQuestions = fopen(QUESTIONS_FILE, "r");
    FILE *fAnswers = fopen(ANSWERS_FILE, "r");

    if (!fUsers || !fPass || !fQuestions || !fAnswers) {
        inicializarUsuarios();  // Si no existen los archivos, inicializa los datos
        return;
    }

    char buffer[100];
    numUsuarios = 0;

    while (fgets(buffer, sizeof(buffer), fUsers)) {
        buffer[strcspn(buffer, "\n")] = 0;  // Eliminar salto de línea
        strcpy(usuarios[numUsuarios].username, buffer);

        fgets(buffer, sizeof(buffer), fPass);
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%*d:%s", usuarios[numUsuarios].password);

        fgets(buffer, sizeof(buffer), fQuestions);
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%*d:%[^\n]", usuarios[numUsuarios].pregunta);

        fgets(buffer, sizeof(buffer), fAnswers);
        buffer[strcspn(buffer, "\n")] = 0;
        sscanf(buffer, "%*d:%[^\n]", usuarios[numUsuarios].respuesta);

        usuarios[numUsuarios].id = numUsuarios + 1;
        usuarios[numUsuarios].activo = true;
        numUsuarios++;
    }

    fclose(fUsers);
    fclose(fPass);
    fclose(fQuestions);
    fclose(fAnswers);
}

bool registrarUsuario() {
    if (numUsuarios >= MAX_USERS) {
        printf("No se pueden registrar más usuarios. Límite alcanzado.\n");
        return false;
    }

    char username[MAX_USER_LENGTH];
    char password[MAX_USER_LENGTH];
    char confirmarPassword[MAX_USER_LENGTH];

    printf("\n=== Registro de Usuario ===\n");
    printf("Nuevo Usuario: ");
    if (!leerString(username, MAX_USER_LENGTH) || !validarUsuario(username)) {
        return false;
    }

    for (int i = 0; i < numUsuarios; i++) {
        if (strcmp(usuarios[i].username, username) == 0) {
            printf("El usuario ya existe. Intente con otro nombre.\n");
            return false;
        }
    }

    printf("Contraseña: ");
    if (!leerString(password, MAX_USER_LENGTH)) {
        printf("Contraseña inválida\n");
        return false;
    }
    if (!validarPassword(password)) {
        return false;
    }

    printf("Confirmar Contraseña: ");
    if (!leerString(confirmarPassword, MAX_USER_LENGTH)) {
        printf("Contraseña inválida\n");
        return false;
    }

    if (strcmp(password, confirmarPassword) != 0) {
        printf("Las contraseñas no coinciden.\n");
        return false;
    }

    // Asignar ID único
    usuarios[numUsuarios].id = numUsuarios > 0 ?
                            usuarios[numUsuarios-1].id + 1 : 1;

    strcpy(usuarios[numUsuarios].username, username);
    strcpy(usuarios[numUsuarios].password, password);

    printf("Pregunta de seguridad: ");
    if (!leerString(usuarios[numUsuarios].pregunta, sizeof(usuarios[numUsuarios].pregunta))) {
        printf("Pregunta inválida.\n");
        return false;
    }

    printf("Respuesta: ");
    if (!leerString(usuarios[numUsuarios].respuesta, sizeof(usuarios[numUsuarios].respuesta))) {
        printf("Respuesta inválida.\n");
        return false;
    }

    usuarios[numUsuarios].activo = true;
    numUsuarios++;
    guardarUsuarios();
    actualizarDiccionario();

    printf("Usuario registrado con éxito.\n");
    return true;
}

bool loginUsuario() {
    const int MAX_INTENTOS = 3;
    int intentos = 0;

    while (intentos < MAX_INTENTOS) {
        char username[MAX_USER_LENGTH] = {0};
        char password[MAX_USER_LENGTH] = {0};

        printf("\n=== Inicio de Sesión ===\n");
        printf("Intento %d/%d\n", intentos + 1, MAX_INTENTOS);

        printf("Usuario: ");
        if (!leerString(username, MAX_USER_LENGTH)) {
            printf("Nombre de usuario inválido\n");
            intentos++;
            continue;
        }

        printf("Password: ");
        if (!leerString(password, MAX_USER_LENGTH)) {
            printf("Contraseña inválida\n");
            intentos++;
            continue;
        }

        for (int i = 0; i < numUsuarios; i++) {
            if (usuarios[i].activo &&
                strcmp(usuarios[i].username, username) == 0 &&
                strcmp(usuarios[i].password, password) == 0) {
                printf("\nAcceso concedido. Bienvenido %s!\n", username);
                return true;
            }
        }

        printf("\nAcceso denegado. Usuario o contraseña incorrectos.\n");
        intentos++;
    }

    printf("\nDemasiados intentos fallidos. Por favor intente más tarde.\n");
    return false;
}

bool recuperarContrasena() {
    char username[MAX_USER_LENGTH];
    char respuesta[100];
    int userIndex = -1;

    printf("\n=== Recuperación de Contraseña ===\n");
    printf("Usuario: ");
    if (!leerString(username, MAX_USER_LENGTH)) {
        printf("Nombre de usuario inválido\n");
        return false;
    }

    // Buscar usuario
    for (int i = 0; i < numUsuarios; i++) {
        if (strcmp(usuarios[i].username, username) == 0) {
            userIndex = i;
            break;
        }
    }

    if (userIndex == -1) {
        printf("Usuario no encontrado.\n");
        return false;
    }

    // Mostrar pregunta de seguridad
    printf("Pregunta de seguridad: %s\n", usuarios[userIndex].pregunta);
    printf("Respuesta: ");
    if (!leerString(respuesta, sizeof(respuesta))) {
        printf("Respuesta inválida\n");
        return false;
    }

    // Verificar respuesta
    if (strcmp(usuarios[userIndex].respuesta, respuesta) != 0) {
        printf("Respuesta incorrecta.\n");
        return false;
    }

    // Cambiar contraseña
    char newPassword[MAX_USER_LENGTH];
    char confirmPassword[MAX_USER_LENGTH];

    printf("Nueva contraseña: ");
    if (!leerString(newPassword, MAX_USER_LENGTH)) {
        printf("Contraseña inválida\n");
        return false;
    }
    if (!validarPassword(newPassword)) {
        return false;
    }

    printf("Confirmar contraseña: ");
    if (!leerString(confirmPassword, MAX_USER_LENGTH)) {
        printf("Contraseña inválida\n");
        return false;
    }

    if (strcmp(newPassword, confirmPassword) != 0) {
        printf("Las contraseñas no coinciden.\n");
        return false;
    }

    // Actualizar contraseña
    strcpy(usuarios[userIndex].password, newPassword);
    guardarUsuarios();

    printf("Contraseña actualizada con éxito.\n");
    return true;
}

// Nueva función para borrar usuario
bool borrarUsuario() {
    char username[MAX_USER_LENGTH];
    char respuesta[100];

    printf("\n=== Borrar Usuario ===\n");
    printf("Usuario a borrar: ");
    if (!leerString(username, MAX_USER_LENGTH)) {
        printf("Nombre de usuario inválido\n");
        return false;
    }

    int posicion = -1;
    for (int i = 0; i < numUsuarios; i++) {
        if (strcmp(usuarios[i].username, username) == 0) {
            posicion = i;
            break;
        }
    }

    if (posicion == -1) {
        printf("Usuario no encontrado.\n");
        return false;
    }

    // Verificar pregunta de seguridad
    printf("Pregunta de seguridad: %s\n", usuarios[posicion].pregunta);
    printf("Respuesta: ");
    if (!leerString(respuesta, sizeof(respuesta))) {
        printf("Respuesta inválida\n");
        return false;
    }

    if (strcmp(usuarios[posicion].respuesta, respuesta) != 0) {
        printf("Respuesta incorrecta.\n");
        return false;
    }

    // Mover usuarios una posición arriba
    for (int i = posicion; i < numUsuarios - 1; i++) {
        usuarios[i] = usuarios[i + 1];  // Copia toda la estructura
    }

    numUsuarios--;
    guardarUsuarios();
    actualizarDiccionario();  // Llamada correcta
    printf("Usuario borrado con éxito.\n");
    return true;
}

// Modificar la función de validación de usuario
bool validarUsuario(const char* username) {
    size_t len = strlen(username);

    if (len < 1 || len > 8) {
        printf("El usuario debe tener entre 1 y 8 caracteres.\n");
        return false;
    }

    return true;
}

// Agregar después de validarUsuario()
bool validarPassword(const char* password) {
    size_t len = strlen(password);
    if (len < 8) {
        printf("La contraseña debe tener al menos 8 caracteres.\n");
        return false;
    }
    return true;
}

// ==================== Funciones para Ordenamiento ====================
int comparar(int a, int b) {
    return a - b;
}

void mostrarArreglo(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void mostrarMetricas(struct Metricas m) {
    printf("\n=== Métricas de Rendimiento ===\n");
    printf("Iteraciones: %d\n", m.iteraciones);
    printf("Consultas: %d\n", m.consultas);
    printf("Movimientos: %d\n", m.movimientos);
    printf("Tiempo: %.4f segundos\n\n", m.tiempo);  // Faltaba el m.tiempo
}

void bubbleSort(int arr[], int n, struct Metricas* m) {
    int j;
    for (int i = 0; i < n - 1; i++) {
        m->iteraciones++;
        for (j = 0; j < n - 1 - i; j++) {
            m->consultas++;
            if (comparar(arr[j], arr[j + 1]) > 0) {
                int aux = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = aux;
                m->movimientos++;
            }
        }
    }
}

void burbujaOptimizado(int arr[], int n, struct Metricas* m) {
    bool intercambio;
    for (int i = 0; i < n - 1; i++) {
        m->iteraciones++;
        intercambio = false;
        for (int j = 0; j < n - i - 1; j++) {
            m->consultas++;
            if (arr[j] > arr[j + 1]) {
                int aux = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = aux;
                m->movimientos++;
                intercambio = true;
            }
        }
        if (!intercambio) break;
    }
}

void selectionSort(int arr[], int n, struct Metricas* m) {
    for (int i = 0; i < n - 1; i++) {
        m->iteraciones++;
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            m->consultas++;
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            int aux = arr[min_idx];
            arr[min_idx] = arr[i];
            arr[i] = aux;
            m->movimientos++;
        }
    }
}

void insertionSort(int arr[], int n, struct Metricas* m) {
    for (int i = 1; i < n; i++) {
        m->iteraciones++;
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            m->consultas++;
            arr[j + 1] = arr[j];
            m->movimientos++;
            j--;
        }
        arr[j + 1] = key;
    }
}

void shellSort(int arr[], int n, struct Metricas* m) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
        m->iteraciones++;
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                m->consultas++;
                arr[j] = arr[j - gap];
                m->movimientos++;
            }
            arr[j] = temp;
        }
    }
}

// Agregar después de las otras funciones de ordenamiento
int partition(int arr[], int low, int high, struct Metricas* m) {
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        m->consultas++;
        if (arr[j] < pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            m->movimientos++;
        }
    }
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    m->movimientos++;

    return (i + 1);
}

void quickSortRecursivo(int arr[], int low, int high, struct Metricas* m) {
    if (low < high) {
        m->iteraciones++;
        int pi = partition(arr, low, high, m);
        quickSortRecursivo(arr, low, pi - 1, m);
        quickSortRecursivo(arr, pi + 1, high, m);
    }
}

void quickSort(int arr[], int n, struct Metricas* m) {
    quickSortRecursivo(arr, 0, n - 1, m);
}

// Agregar después de las otras funciones de ordenamiento

void merge(int arr[], int left, int mid, int right, struct Metricas* m) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Crear arreglos temporales
    int L[n1], R[n2];

    // Copiar datos a arreglos temporales
    for (i = 0; i < n1; i++) {
        L[i] = arr[left + i];
        m->movimientos++;
    }
    for (j = 0; j < n2; j++) {
        R[j] = arr[mid + 1 + j];
        m->movimientos++;
    }

    // Fusionar los arreglos temporales
    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2) {
        m->consultas++;
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        m->movimientos++;
        k++;
    }

    // Copiar elementos restantes de L[]
    while (i < n1) {
        arr[k] = L[i];
        m->movimientos++;
        i++;
        k++;
    }

    // Copiar elementos restantes de R[]
    while (j < n2) {
        arr[k] = R[j];
        m->movimientos++;
        j++;
        k++;
    }
}

void mergeSortRecursivo(int arr[], int left, int right, struct Metricas* m) {
    if (left < right) {
        m->iteraciones++;
        int mid = left + (right - left) / 2;

        mergeSortRecursivo(arr, left, mid, m);
        mergeSortRecursivo(arr, mid + 1, right, m);

        merge(arr, left, mid, right, m);
    }
}

void mergeSort(int arr[], int n, struct Metricas* m) {
    mergeSortRecursivo(arr, 0, n - 1, m);
}

// Agregar después de las otras funciones de ordenamiento
void combSort(int arr[], int n, struct Metricas* m);

// Modificar la función mostrarMenu()
void mostrarMenu() {
    printf("\n+--------------------------------+\n");
    printf("|        Menú de Opciones        |\n");
    printf("+--------------------------------+\n");
    printf("| 1. Métodos de Ordenamiento     |\n");
    printf("| 2. Árboles                     |\n");
    printf("| 3. Estructuras LIFO/FIFO       |\n");
    printf("| 4. Ver Diccionario             |\n");
    printf("| 5. Salir                       |\n");
    printf("+--------------------------------+\n");
    printf("Ingrese su opción: ");
}

void mostrarMenuOrdenamiento() {
    printf("\n+-------------------------+\n");
    printf("|  Métodos de Ordenamiento|\n");
    printf("+-------------------------+\n");
    printf("| 1. Bubble Sort         |\n");
    printf("| 2. Burbuja Optimizado  |\n");
    printf("| 3. Selection Sort      |\n");
    printf("| 4. Insertion Sort      |\n");
    printf("| 5. Shell Sort          |\n");
    printf("| 6. Quick Sort          |\n");
    printf("| 7. Merge Sort          |\n");
    printf("| 8. Comb Sort           |\n");
    printf("| 9. Cocktail Sort       |\n");
    printf("| 10. Volver             |\n");
    printf("+-------------------------+\n");
    printf("Ingrese su opción: ");
}

void mostrarMenuArboles() {
    printf("\n+-------------------------+\n");
    printf("|    Métodos de Árboles   |\n");
    printf("+-------------------------+\n");
    printf("| 1. Árbol AVL           |\n");
    printf("| 2. Volver              |\n");
    printf("+-------------------------+\n");
    printf("Ingrese su opción: ");
}

void mostrarMenuPrincipal() {
    printf("\n+--------------------------------+\n");
    printf("|         Menu Principal         |\n");
    printf("+--------------------------------+\n");
    printf("| 1. Iniciar Sesion             |\n");
    printf("| 2. Registrar Usuario          |\n");
    printf("| 3. Borrar Usuario             |\n");
    printf("| 4. Salir                      |\n");
    printf("+--------------------------------+\n");
    printf("Ingrese su opcion: ");
}

// ==================== Función Principal ====================
int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8); // Configurar salida en UTF-8
        SetConsoleCP(CP_UTF8);       // Configurar entrada en UTF-8
    #endif

    cargarUsuarios();
    int opcionInicio, opcionMenu, opcionSubMenu;
    struct Metricas metricas;

    do {
        mostrarMenuPrincipal();
        if (!leerEntero(&opcionInicio)) {
            printf("Opción inválida. Intente nuevamente.\n");
            limpiarBuffer();
            continue;
        }

        switch (opcionInicio) {
            case 1:
                if (loginUsuario()) {
                    do {
                        mostrarMenu();
                        if (!leerEntero(&opcionMenu)) {
                            printf("Opción inválida. Intente nuevamente.\n");
                            limpiarBuffer();
                            continue;
                        }

                        switch (opcionMenu) {
                            case 1: // Métodos de Ordenamiento
                                do {
                                    mostrarMenuOrdenamiento();
                                    if (!leerEntero(&opcionSubMenu)) {
                                        printf("Opción inválida.\n");
                                        continue;
                                    }

                                    if (opcionSubMenu >= 1 && opcionSubMenu <= 9) {
                                        int n;
                                        printf("Ingrese el tamaño del arreglo: ");
                                        if (!leerEntero(&n) || n <= 0 || n > MAX_SIZE) {
                                            printf("Tamaño inválido.\n");
                                            continue;
                                        }

                                        int arr[MAX_SIZE];
                                        printf("Ingrese los elementos:\n");
                                        for (int i = 0; i < n; i++) {
                                            if (!leerEntero(&arr[i])) {
                                                printf("Valor inválido.\n");
                                                i--;
                                                continue;
                                            }
                                        }

                                        memset(&metricas, 0, sizeof(struct Metricas));
                                        clock_t inicio = clock();

                                        switch (opcionSubMenu) {
                                            case 1: bubbleSort(arr, n, &metricas); break;
                                            case 2: burbujaOptimizado(arr, n, &metricas); break;
                                            case 3: selectionSort(arr, n, &metricas); break;
                                            case 4: insertionSort(arr, n, &metricas); break;
                                            case 5: shellSort(arr, n, &metricas); break;
                                            case 6: quickSort(arr, n, &metricas); break;
                                            case 7: mergeSort(arr, n, &metricas); break;
                                            case 8: combSort(arr, n, &metricas); break;
                                            case 9: cocktailSort(arr, n, &metricas); break;
                                        }

                                        metricas.tiempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
                                        printf("\nArreglo ordenado:\n");
                                        mostrarArreglo(arr, n);
                                        mostrarMetricas(metricas);
                                    }
                                } while (opcionSubMenu != 10);
                                break;

                            case 2: // Árboles
                                do {
                                    mostrarMenuArboles();
                                    if (!leerEntero(&opcionSubMenu)) {
                                        printf("Opción inválida.\n");
                                        continue;
                                    }

                                    if (opcionSubMenu == 1) {
                                        int n;
                                        printf("Ingrese el tamaño del arreglo para el árbol AVL: ");
                                        if (!leerEntero(&n) || n <= 0 || n > MAX_SIZE) {
                                            printf("Tamaño inválido.\n");
                                            continue;
                                        }

                                        int arr[MAX_SIZE];
                                        printf("Ingrese los elementos:\n");
                                        for (int i = 0; i < n; i++) {
                                            if (!leerEntero(&arr[i])) {
                                                printf("Valor inválido.\n");
                                                i--;
                                                continue;
                                            }
                                        }

                                        memset(&metricas, 0, sizeof(struct Metricas));
                                        clock_t inicio = clock();
                                        avlSort(arr, n, &metricas);
                                        metricas.tiempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
                                        mostrarMetricas(metricas);
                                    }
                                } while (opcionSubMenu != 2);
                                break;

                            case 3: // Estructuras LIFO/FIFO
                                do {
                                    mostrarMenuEstructuras();
                                    if (!leerEntero(&opcionSubMenu)) {
                                        printf("Opción inválida.\n");
                                        continue;
                                    }

                                    struct Stack pila;
                                    struct Queue cola;
                                    memset(&metricas, 0, sizeof(struct Metricas));

                                    switch (opcionSubMenu) {
                                        case 1: // LIFO
                                            initStack(&pila);
                                            // Implementar operaciones de pila
                                            break;
                                        case 2: // FIFO
                                            initQueue(&cola);
                                            // Implementar operaciones de cola
                                            break;
                                    }
                                } while (opcionSubMenu != 3);
                                break;

                            case 4:
                                mostrarDiccionario();
                                break;
                        }
                    } while (opcionMenu != 5);
                }
                break;
            case 2:
                registrarUsuario();
                break;
            case 3:
                borrarUsuario();
                break;
            case 4:
                printf("Programa terminado.\n");
                break;
            default:
                printf("Opción no válida.\n");
        }
    } while (opcionInicio != 4);

    return 0;
}

void limpiarBuffer() {
    char temp[MAX_INPUT];
    size_t len = 0;
    do {
        fgets(temp, sizeof(temp), stdin);
        len = strlen(temp);
    } while (len == MAX_INPUT - 1 && temp[len-1] != '\n');
}

bool leerEntero(int* numero) {
    char input[100];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return false;
    }

    // Eliminar el salto de línea final si existe
    size_t len = strlen(input);
    if (len > 0 && input[len-1] == '\n') {
        input[len-1] = '\0';
        len--;
    }

    // Verificar si está vacío
    if (len == 0) {
        return false;
    }

    // Manejar el signo negativo
    int i = 0;
    if (input[0] == '-' || input[0] == '+') {
        if (len == 1) return false; // Solo signo sin número
        i = 1;
    }

    // Verificar que el resto sean dígitos
    for (; i < len; i++) {
        if (!isdigit(input[i])) {
            printf("Por favor ingrese solo números (pueden ser negativos)\n");
            return false;
        }
    }

    // Convertir a número usando strtol para mejor manejo de errores
    char* endptr;
    long valor = strtol(input, &endptr, 10);

    // Verificar desbordamiento
    if (valor > INT_MAX || valor < INT_MIN) {
        printf("El número está fuera del rango permitido\n");
        return false;
    }

    *numero = (int)valor;
    return true;
}

// Mejorar la función leerString para validar mejor la entrada
bool leerString(char* str, int maxLen) {
    if (!str || maxLen <= 0) return false;

    memset(str, 0, maxLen);
    char* result = fgets(str, maxLen, stdin);

    if (result == NULL) {
        return false;
    }

    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
        len--;
    }

    if (len == 0) {
        return false;
    }

    // Validar caracteres
    for (size_t i = 0; i < len; i++) {
        if (!isprint(str[i])) {
            return false;
        }
    }

    return true;
}

// Modificar la definición de actualizarDiccionario
void actualizarDiccionario(void) {
    for(int i = 0; i < numUsuarios && i < MAX_USERS; i++) {
        int fila = i / MATRIX_SIZE;
        int col = i % MATRIX_SIZE;

        if (fila >= MATRIX_SIZE || col >= MATRIX_SIZE) {
            continue;  // Protección contra desbordamiento
        }

        diccionario[i].id = usuarios[i].id;
        diccionario[i].fila = fila;
        diccionario[i].columna = col;
        snprintf(diccionario[i].valor, MAX_USER_LENGTH,
                "ID: %d, Pos[%d,%d], User: %s",
                usuarios[i].id, fila, col,
                usuarios[i].username);

        // Actualizar matrices
        matrizUsuarios[fila][col] = usuarios[i].username[0];
        matrizPasswords[fila][col] = usuarios[i].password[0];
    }
}

// Modificar la definición de mostrarDiccionario
void mostrarDiccionario(void) {
    printf("\n=== Diccionario de Datos ===\n");
    for(int i = 0; i < numUsuarios; i++) {
        printf("Entrada %d: %s\n", i+1, diccionario[i].valor);
    }
    printf("\n");
}

// Agregar con las otras funciones de ordenamiento
void combSort(int arr[], int n, struct Metricas* m) {
    if (!arr || !m || n <= 0) return;

    int gap = n;
    bool swapped = true;
    const float shrink = 1.3;

    while (gap > 1 || swapped) {
        m->iteraciones++;
        gap = (gap > 1) ? (int)(gap / shrink) : 1;
        swapped = false;

        for (int i = 0; i + gap < n; i++) {
            m->consultas++;
            if (arr[i] > arr[i + gap]) {
                int temp = arr[i];
                arr[i] = arr[i + gap];
                arr[i + gap] = temp;
                m->movimientos++;
                swapped = true;
            }
            mostrarArreglo(arr, n);
        }
    }
}

// Implementar las funciones del árbol AVL
struct NodoAVL* newNodo(int dato) {
    struct NodoAVL* nodo = (struct NodoAVL*)malloc(sizeof(struct NodoAVL));
    if (nodo == NULL) {
        printf("Error: No se pudo asignar memoria\n");
        return NULL;
    }
    nodo->dato = dato;
    nodo->izquierda = NULL;
    nodo->derecha = NULL;
    nodo->altura = 1;
    return nodo;
}

int altura(struct NodoAVL* N) {
    if (N == NULL)
        return 0;
    return N->altura;
}

int maximo(int a, int b) {
    return (a > b) ? a : b;
}

int getBalance(struct NodoAVL* N) {
    if (N == NULL)
        return 0;
    return altura(N->izquierda) - altura(N->derecha);
}

struct NodoAVL* rotarDerecha(struct NodoAVL* y) {
    struct NodoAVL* x = y->izquierda;
    struct NodoAVL* T2 = x->derecha;

    x->derecha = y;
    y->izquierda = T2;

    y->altura = max(altura(y->izquierda), altura(y->derecha)) + 1;
    x->altura = max(altura(x->izquierda), altura(x->derecha)) + 1;

    return x;
}

struct NodoAVL* rotarIzquierda(struct NodoAVL* x) {
    struct NodoAVL* y = x->derecha;
    struct NodoAVL* T2 = y->izquierda;

    y->izquierda = x;
    x->derecha = T2;

    x->altura = max(altura(x->izquierda), altura(x->derecha)) + 1;
    y->altura = max(altura(y->izquierda), altura(y->derecha)) + 1;

    return y;
}

struct NodoAVL* insertar(struct NodoAVL* nodo, int dato, struct Metricas* m) {
    m->iteraciones++;

    if (nodo == NULL)
        return newNodo(dato);

    m->consultas++;
    if (dato < nodo->dato)
        nodo->izquierda = insertar(nodo->izquierda, dato, m);
    else if (dato > nodo->dato)
        nodo->derecha = insertar(nodo->derecha, dato, m);
    else
        return nodo;  // No permitir duplicados

    // Actualizar altura
    nodo->altura = 1 + max(altura(nodo->izquierda), altura(nodo->derecha));

    // Obtener factor de balance
    int balance = getBalance(nodo);
    m->consultas++;

    // Casos de balanceo
    // Rotación simple derecha
    if (balance > 1 && dato < nodo->izquierda->dato) {
        m->movimientos++;
        return rotarDerecha(nodo);
    }
    // Rotación simple izquierda
    if (balance < -1 && dato > nodo->derecha->dato) {
        m->movimientos++;
        return rotarIzquierda(nodo);
    }
    // Rotación doble izquierda-derecha
    if (balance > 1 && dato > nodo->izquierda->dato) {
        m->movimientos += 2;
        nodo->izquierda = rotarIzquierda(nodo->izquierda);
        return rotarDerecha(nodo);
    }
    // Rotación doble derecha-izquierda
    if (balance < -1 && dato < nodo->derecha->dato) {
        m->movimientos += 2;
        nodo->derecha = rotarDerecha(nodo->derecha);
        return rotarIzquierda(nodo);
    }

    return nodo;
}

void inOrder(struct NodoAVL* raiz) {
    if (raiz != NULL) {
        inOrder(raiz->izquierda);
        printf("%d ", raiz->dato);
        inOrder(raiz->derecha);
    }
}

void liberarArbol(struct NodoAVL* raiz) {
    if (raiz != NULL) {
        liberarArbol(raiz->izquierda);
        liberarArbol(raiz->derecha);
        free(raiz);
    }
}

// Mejorar la función avlSort para manejar errores de memoria
void avlSort(int arr[], int n, struct Metricas* m) {
    if (!arr || !m || n <= 0) {
        printf("Parámetros inválidos para avlSort\n");
        return;
    }

    struct NodoAVL* raiz = NULL;
    bool error = false;

    printf("\nProceso de construcción del árbol AVL:\n");
    printf("=====================================\n");

    // Insertar elementos y mostrar el árbol después de cada inserción
    for(int i = 0; i < n; i++) {
        printf("\nInsertando %d:\n", arr[i]);
        printf("-----------------\n");
        raiz = insertar(raiz, arr[i], m);
        mostrarArbolAVL(raiz);
    }

    // Recorrer el árbol en orden para obtener elementos ordenados
    printf("\nRecorrido en orden (elementos ordenados):\n");
    int i = 0;
    struct NodoAVL* actual = raiz;
    struct NodoAVL* pila[MAX_SIZE];  // Cambiado de n a MAX_SIZE para evitar VLA
    int top = -1;

    // Recorrido iterativo en orden
    while (actual != NULL || top >= 0) {
        // Llegar al nodo más a la izquierda
        while (actual != NULL) {
            pila[++top] = actual;
            actual = actual->izquierda;
        }

        // Procesar el nodo actual y moverse a la derecha
        actual = pila[top--];
        arr[i++] = actual->dato;
        m->movimientos++;
        actual = actual->derecha;
    }

    // Liberar memoria
    liberarArbol(raiz);
}

// Modificar las funciones de visualización del árbol
void imprimirArbol(struct NodoAVL* raiz, int espacio) {
    if (raiz == NULL)
        return;

    // Aumentar la distancia entre niveles
    espacio += 10;

    // Procesar hijo derecho primero (arriba)
    imprimirArbol(raiz->derecha, espacio);

    // Imprimir nodo actual
    printf("\n");
    for (int i = 10; i < espacio; i++)
        printf(" ");
    printf("%d\n", raiz->dato);

    // Procesar hijo izquierdo (abajo)
    imprimirArbol(raiz->izquierda, espacio);
}

void mostrarArbolAVL(struct NodoAVL* raiz) {
    if (raiz == NULL) {
        printf("Árbol vacío\n");
        return;
    }

    printf("\nEstructura del Árbol AVL:\n");
    printf("========================\n");

    int altura = obtenerAltura(raiz);
    int espacioInicial = 4;  // Reducido para mejor visualización

    for (int i = 0; i < altura; i++) {
        printf("\nNivel %d: ", i);
        imprimirNivel(raiz, i, espacioInicial * (1 << (altura - i - 1)));
    }
    printf("\n");
}

void imprimirNivel(struct NodoAVL* raiz, int nivel, int espacio) {
    if (raiz == NULL) {
        for (int i = 0; i < espacio; i++) printf(" ");
        printf("--");
        return;
    }

    if (nivel == 0) {
        for (int i = 0; i < espacio; i++) printf(" ");
        printf("%2d", raiz->dato);
    } else {
        imprimirNivel(raiz->izquierda, nivel - 1, espacio);
        imprimirNivel(raiz->derecha, nivel - 1, espacio);
    }
}

int obtenerAltura(struct NodoAVL* raiz) {
    if (raiz == NULL) return 0;
    int alturaIzq = obtenerAltura(raiz->izquierda);
    int alturaDer = obtenerAltura(raiz->derecha);
    return (alturaIzq > alturaDer) ? alturaIzq : alturaDer;
}

void imprimirArbolMejorado(struct NodoAVL* raiz, int nivel, const char* prefijo, bool esIzquierdo) {
    if (raiz == NULL) return;

    printf("%s%s%d\n", prefijo, esIzquierdo ? "├── " : "└── ", raiz->dato);

    char nuevoPrefijo[256];
    snprintf(nuevoPrefijo, sizeof(nuevoPrefijo), "%s%s", prefijo, esIzquierdo ? "│   " : "    ");

    imprimirArbolMejorado(raiz->izquierda, nivel + 1, nuevoPrefijo, true);
    imprimirArbolMejorado(raiz->derecha, nivel + 1, nuevoPrefijo, false);
}

void cocktailSort(int arr[], int n, struct Metricas* m) {
    bool swapped = true;
    int start = 0;
    int end = n - 1;
    int iteraciones_completas = 0;

    while (swapped) {
        swapped = false;
        m->iteraciones++;

        // Pasada izquierda a derecha
        printf("\nIteración %d (izq->der): ", iteraciones_completas + 1);
        for (int i = start; i < end; i++) {
            m->consultas++;
            if (arr[i] > arr[i + 1]) {
                int temp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = temp;
                m->movimientos++;
                swapped = true;
            }
            // Mostrar estado actual del arreglo
            mostrarArreglo(arr, n);
        }

        if (!swapped) break;

        swapped = false;
        end--;

        // Pasada derecha a izquierda
        printf("\nIteración %d (der->izq): ", iteraciones_completas + 1);
        for (int i = end - 1; i >= start; i--) {
            m->consultas++;
            if (arr[i] > arr[i + 1]) {
                int temp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = temp;
                m->movimientos++;
                swapped = true;
            }
            // Mostrar estado actual del arreglo
            mostrarArreglo(arr, n);
        }

        start++;
        iteraciones_completas++;
    }

    printf("\n=== Estadísticas de Ordenamiento ===\n");
    printf("Iteraciones completas: %d\n", iteraciones_completas);
    printf("Total pasadas: %d\n", m->iteraciones * 2); // Dos pasadas por iteración
    printf("Comparaciones: %d\n", m->consultas);
    printf("Movimientos (swaps): %d\n", m->movimientos);
}

// ...existing code...

void mostrarMenuEstructuras(void) {
    printf("\n+--------------------------------+\n");
    printf("|    Estructuras LIFO/FIFO       |\n");
    printf("+--------------------------------+\n");
    printf("| 1. Operaciones LIFO (Pila)     |\n");
    printf("| 2. Operaciones FIFO (Cola)     |\n");
    printf("| 3. Volver                      |\n");
    printf("+--------------------------------+\n");
    printf("Ingrese su opción: ");
}

void initStack(struct Stack* s) {
    if (s == NULL) return;
    s->top = -1;  // Inicializa la pila vacía
}

void initQueue(struct Queue* q) {
    if (q == NULL) return;
    q->front = -1;  // Inicializa la cola vacía
    q->rear = -1;
}

bool isStackEmpty(struct Stack* s) {
    return (s->top == -1);
}

bool isStackFull(struct Stack* s) {
    return (s->top == MAX_STACK - 1);
}

bool isQueueEmpty(struct Queue* q) {
    return (q->front == -1);
}

bool isQueueFull(struct Queue* q) {
    return ((q->rear + 1) % MAX_QUEUE == q->front);
}

void push(struct Stack* s, int value, struct Metricas* m) {
    if (isStackFull(s)) {
        printf("Error: Pila llena\n");
        return;
    }
    s->items[++s->top] = value;
    m->movimientos++;
    printf("Elemento %d insertado en la pila\n", value);
}

int pop(struct Stack* s, struct Metricas* m) {
    if (isStackEmpty(s)) {
        printf("Error: Pila vacía\n");
        return -1;
    }
    m->movimientos++;
    return s->items[s->top--];
}

void enqueue(struct Queue* q, int value, struct Metricas* m) {
    if (isQueueFull(q)) {
        printf("Error: Cola llena\n");
        return;
    }
    if (q->front == -1) {
        q->front = 0;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE;
    q->items[q->rear] = value;
    m->movimientos++;
    printf("Elemento %d insertado en la cola\n", value);
}

int dequeue(struct Queue* q, struct Metricas* m) {
    if (isQueueEmpty(q)) {
        printf("Error: Cola vacía\n");
        return -1;
    }
    int item = q->items[q->front];
    if (q->front == q->rear) {
        // Último elemento en la cola
        q->front = -1;
        q->rear = -1;
    } else {
        q->front = (q->front + 1) % MAX_QUEUE;
    }
    m->movimientos++;
    return item;
}
