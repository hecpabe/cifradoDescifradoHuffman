

/*
    Título: Cifrar
    Nombre: Héctor Paredes Benavides
    Descripción: Creamos un programa para cifrar una cadena de caracteres de longitud variable con el algoritmo de Huffman
    Fecha: 4/12/2022
*/

/* Instrucciones de Preprocesado */
// Inclusión de bibliotecas externas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definición de constantes
#define HASH_TABLE_SIZE 39
#define BITS_IN_BYTE 8
#define FREQUENCY_TABLE_FILE "frequency.txt"
#define TREE_FILE "tree.txt"
#define HUFFMAN_CODES_FILE "codes.txt"
#define ENCODED_FILE "compressed.bin"

#define byte char

/* Declaraciones Globales */
// Estructuras
typedef struct FileLine_s{

    int lineLength;
    char *lineContent;

}FileLine_s;

typedef struct FileContent_s{

    int linesNumber;
    FileLine_s *fileLines;

}FileContent_s;

typedef struct HashTable_s{

    char key;
    unsigned int value;

}HashTable_s;

typedef struct StringCharacter_s{

    char character;
    int frequency;

}StringCharacter_s;

typedef struct LinkedListNode_s{

    StringCharacter_s stringCharacter;
    struct LinkedListNode_s *nextNode;
    struct LinkedListNode_s *backNode;

}LinkedListNode_s;

typedef struct TreeNode_s{

    StringCharacter_s stringCharacter;
    struct TreeNode_s *parentNode;
    struct TreeNode_s *leftChild;
    struct TreeNode_s *rightChild;

}TreeNode_s;

typedef struct HuffmanCode_s{

    char character;
    char *code;
    int codeLength;

}HuffmanCode_s;

// Prototipado de Funciones
// Funciones Lista Enlazada
LinkedListNode_s* initLinkedListFromFrequencyTable(HashTable_s *frequencyTable);
void insertElementInPriorityQueue(LinkedListNode_s **queue, char character, int frequency);
void printLinkedList(char *fileName, LinkedListNode_s *linkedList);
void freeLinkedList(LinkedListNode_s *linkedList);

// Funciones Árboles
TreeNode_s* initTreeFromPriorityQueue(LinkedListNode_s *queue);
TreeNode_s* buildTree(TreeNode_s **nodes, int nodesLength);
TreeNode_s* findMinNode(TreeNode_s **nodes, int *nodesLength);
void printTree(FILE *file, TreeNode_s *tree);
void freeTree(TreeNode_s *tree);

// Funciones algoritmo de Huffman
HuffmanCode_s* initHuffmanCodes();
void generateHuffmanCodes(HuffmanCode_s **huffmanCodes, TreeNode_s *huffmanTree, int *currentCode, int depth, int *maxDepth);
void printHuffmanCodes(char *fileName, LinkedListNode_s *charactersList, HuffmanCode_s *huffmanCodes);
byte* encodeFileContent(FileContent_s fileContent, HuffmanCode_s *huffmanCodes, int maxCodeLength, int *bytesLength);
void printEncodedFileContent(char *fileName, byte *encodedFileContent, int length);

// Funciones tabla hash
HashTable_s* initHashTable();
int getHash(char key);

// Funciones auxiliares
char* readLine(int *length);
FileContent_s readFileContent(char *fileName);
FileLine_s readFileLine(FILE *file);
void freeFileContent(FileContent_s fileContent);

/* Función Principal Main*/
int main(int argc, char **argv){

    // Variables necesarias
    char *fileName = NULL;
    int fileNameLength = 0;
    FileContent_s fileContent;
    HashTable_s *frequencyTable = NULL;
    LinkedListNode_s *priorityQueue;
    TreeNode_s *charactersTree = NULL;
    FILE *treeFile = NULL;
    HuffmanCode_s *huffmanCodes = NULL;
    int huffmanCodesMaxLength = 0;
    byte *encodedFileContent = NULL;
    int encodedFileContentLength = 0;

    // Obtenemos el nombre del archivo
    printf("Introduzca el nombre del fichero a cifrar: ");
    fileName = readLine(&fileNameLength);

    // Abrimos el fichero y leemos su contenido
    fileContent = readFileContent(fileName);

    // Inicializamos la tabla de frecuencias
    frequencyTable = initHashTable();

    // Recorremos el contenido del fichero y establecemos la tabla de frecuencias correspondiente
    for(int i = 0; i < fileContent.linesNumber; i++)
        for(int j = 0; j < fileContent.fileLines[i].lineLength; j++)
            frequencyTable[getHash(fileContent.fileLines[i].lineContent[j])].value += 1;

    // Obtenemos la tabla de frecuencias en forma de cola de prioridad
    priorityQueue = initLinkedListFromFrequencyTable(frequencyTable);

    // Imprimimos el contenido de la tabla de frecuencias en el fichero correspondiente
    printLinkedList(FREQUENCY_TABLE_FILE, priorityQueue);

    // Creamos el árbol con los nodos de las letras
    charactersTree = initTreeFromPriorityQueue(priorityQueue);

    // Imprimimos el árbol de huffman en el fichero 
    treeFile = fopen(TREE_FILE, "w");

    // Comprobamos que el fichero se haya abierto correctamente
    if(treeFile == NULL){

        printf("ERROR: Ha ocurrido un error al intentar abrir el fichero '%s'.\n", fileName);
        exit(1);

    }

    printTree(treeFile, charactersTree);

    fclose(treeFile);

    // Creamos la tabla hash para los códigos huffman
    huffmanCodes = initHuffmanCodes();
    generateHuffmanCodes(&huffmanCodes, charactersTree, NULL, 0, &huffmanCodesMaxLength);
    printHuffmanCodes(HUFFMAN_CODES_FILE, priorityQueue, huffmanCodes);

    // Obtenemos el contenido del fichero codificado
    encodedFileContent = encodeFileContent(fileContent, huffmanCodes, huffmanCodesMaxLength, &encodedFileContentLength);
    printEncodedFileContent(ENCODED_FILE, encodedFileContent, encodedFileContentLength);

    /* printf("File Content:\n");
    for(int i = 0; i < fileContent.linesNumber; i++)
        printf("Line %d (%d): %s\n", i + 1, fileContent.fileLines[i].lineLength, fileContent.fileLines[i].lineContent);

    printf("Tabla de frecuencias:\n");
    for(int i = 0; i < HASH_TABLE_SIZE; i++)
        printf("%c -> %d\n", frequencyTable[i].key, frequencyTable[i].value); */

    // Liberamos la memoria utilizada
    freeFileContent(fileContent);
    freeLinkedList(priorityQueue);
    freeTree(charactersTree);

    free(fileName);
    free(frequencyTable);
    free(huffmanCodes);
    free(encodedFileContent);

    return 0;

}

/* Codificación de Funciones */
// initLinkedListFromFrequencyTable
LinkedListNode_s* initLinkedListFromFrequencyTable(HashTable_s *frequencyTable){

    // Variables necesarias
    LinkedListNode_s *linkedListStart = NULL;
    int firstInsertion = 1;

    // Inicializamos la lista
    linkedListStart = (LinkedListNode_s*)malloc(sizeof(LinkedListNode_s));
    linkedListStart->backNode = NULL;
    linkedListStart->nextNode = NULL;
    linkedListStart->stringCharacter.character = '\0';
    linkedListStart->stringCharacter.frequency = 0;

    // Recorremos la tabla hash de frecuencias y vamos introduciendo los caracteres que aparecen al menos una vez
    for(int i = 0; i < HASH_TABLE_SIZE; i++){

        if(frequencyTable[i].value > 0){

            if(firstInsertion){

                firstInsertion = 0;

                linkedListStart->stringCharacter.character = frequencyTable[i].key;
                linkedListStart->stringCharacter.frequency = frequencyTable[i].value;

            }
            else
                insertElementInPriorityQueue(&linkedListStart, frequencyTable[i].key, frequencyTable[i].value);

        }

    }
    
    return linkedListStart;

}

// insertElementInPriorityQueue
void insertElementInPriorityQueue(LinkedListNode_s **queue, char character, int frequency){

    // Variables necesarias
    LinkedListNode_s *queueCopy = NULL;
    LinkedListNode_s *newNode = NULL;

    // Inicializamos la copia de la lista
    queueCopy = *queue;

    // Inicializamos el nuevo nodo
    newNode = (LinkedListNode_s*)malloc(sizeof(LinkedListNode_s));
    newNode->stringCharacter.character = character;
    newNode->stringCharacter.frequency = frequency;

    // Buscamos la posición en la que insertarlo siguiendo una cola de prioridad ascendente
    while(queueCopy->nextNode != NULL && queueCopy->stringCharacter.frequency < newNode->stringCharacter.frequency)
        queueCopy = queueCopy->nextNode;

    // Si tenemos que insertarlo en el inicio
    if(queueCopy->backNode == NULL && queueCopy->stringCharacter.frequency >= newNode->stringCharacter.frequency){

        // Establecemos los vecinos del nuevo nodo
        newNode->backNode = NULL;
        newNode->nextNode = queueCopy;

        // Establecemos al nuevo nodo como vecino
        queueCopy->backNode = newNode;

        // Establecemos al nuevo nodo como origen de la lista
        *queue = newNode;

    }
    // Si tenemos que insertarlo al final
    else if(queueCopy->nextNode == NULL && queueCopy->stringCharacter.frequency < newNode->stringCharacter.frequency){

        // Establecemos los vecinos del nuevo nodo
        newNode->backNode = queueCopy;
        newNode->nextNode = NULL;

        // Establecemos al nuevo nodo como vecino
        queueCopy->nextNode = newNode;

    }
    // Si tenemos que insertarlo en el medio
    else{

        // Establecemos los vecinos del nuevo nodo
        newNode->backNode = queueCopy->backNode;
        newNode->nextNode = queueCopy;

        // Establecemos al nuevo nodo como vecino
        newNode->backNode->nextNode = newNode;
        newNode->nextNode->backNode = newNode;

    }

}

// printLinkedList
void printLinkedList(char *fileName, LinkedListNode_s *linkedList){

    // Variables necesarias
    FILE *file = NULL;
    LinkedListNode_s *linkedListCopy = NULL;

    // Abrimos el fichero
    file = fopen(fileName, "w");

    // Comprobamos que el fichero se haya abierto correctamente
    if(file == NULL){

        printf("ERROR: Ha ocurrido un error al intentar abrir el fichero '%s'.\n", fileName);
        exit(1);

    }

    // Inicializamos la copia de la lista
    linkedListCopy = linkedList;

    while(linkedListCopy != NULL){

        fprintf(file, "'%c' -> %d\n", linkedListCopy->stringCharacter.character, linkedListCopy->stringCharacter.frequency);
        linkedListCopy = linkedListCopy->nextNode;

    }

    // Cerramos el fichero
    fclose(file);

}

// freLinkedList
void freeLinkedList(LinkedListNode_s *linkedList){

    // Avanzamos hasta el final de la lista
    while(linkedList->nextNode != NULL)
        linkedList = linkedList->nextNode;
    
    // Vamos liberando según retrocedemos en los elementos de la lista
    while(linkedList->backNode != NULL){

        linkedList = linkedList->backNode;
        free(linkedList->nextNode);

    }

    // Liberamos el inicio de la lista
    free(linkedList);

}

// initTreeFromPriorityQueue
TreeNode_s* initTreeFromPriorityQueue(LinkedListNode_s *queue){

    // Variables necesarias
    TreeNode_s *treeRoot = NULL;
    TreeNode_s *nodes = NULL;
    LinkedListNode_s *queueCopy = NULL;
    int nodesArrayLength = 0;

    // Inicializamos la copia de la cola
    queueCopy = queue;

    // Transformamos los nodos de la lista de prioridad a nodos de árbol
    while(queueCopy != NULL){

        // Reservamos memoria para el nuevo nodo
        nodes = (TreeNode_s*)realloc(nodes, (nodesArrayLength + 1) * sizeof(TreeNode_s));

        // Establecemos los datos del nuevo nodo e incrementamos el contador de nodos
        nodes[nodesArrayLength].parentNode = NULL;
        nodes[nodesArrayLength].leftChild = NULL;
        nodes[nodesArrayLength].rightChild = NULL;
        nodes[nodesArrayLength].stringCharacter.character = queueCopy->stringCharacter.character;
        nodes[nodesArrayLength].stringCharacter.frequency = queueCopy->stringCharacter.frequency;
        nodesArrayLength++;

        // Avanzamos al siguiente nodo de la cola
        queueCopy = queueCopy->nextNode;

    }

    // Construimos el árbol
    treeRoot = buildTree(&nodes, nodesArrayLength);

    // Liberamos la memoria utilizada
    free(nodes);

    // Devolvemos los datos
    return treeRoot;

}

// buildTree
TreeNode_s* buildTree(TreeNode_s **nodes, int nodesLength){

    // Variables necesarias
    TreeNode_s *rootNode = NULL;
    TreeNode_s *leftNode = NULL;
    TreeNode_s *rightNode = NULL;
    int nodesLengthCopy = 0;

    // Realizamos uan copia de la longitud del array
    nodesLengthCopy = nodesLength;

    // Mientras queden varios nodos en el array
    while(nodesLengthCopy > 1){

        // Extraemos los dos nodos mínimos del array
        leftNode = findMinNode(nodes, &nodesLengthCopy);
        rightNode = findMinNode(nodes, &nodesLengthCopy);

        // Creamos el nodo padre de ambos, lo inicializamos a caracter nulo y establecemos su frecuencia como la suma de ambos hijos
        rootNode = (TreeNode_s*)malloc(sizeof(TreeNode_s));
        rootNode->parentNode = NULL;
        rootNode->stringCharacter.character = '\0';
        rootNode->stringCharacter.frequency = leftNode->stringCharacter.frequency + rightNode->stringCharacter.frequency;

        // Establecemos las relaciones
        rootNode->leftChild = leftNode;
        leftNode->parentNode = rootNode;

        rootNode->rightChild = rightNode;
        rightNode->parentNode = rootNode;

        // Insertamos el nuevo nodo al final del array
        (*nodes)[nodesLengthCopy] = *rootNode;
        nodesLengthCopy++;

    }

    return findMinNode(nodes, &nodesLengthCopy);

}

// findMinNode
TreeNode_s* findMinNode(TreeNode_s **nodes, int *nodesLength){

    // Variables necesarias
    TreeNode_s *minNode = NULL;
    int minNodeIndex = 0;

    // Recorremos la lista de nodos buscando el nodo con menor frecuencia
    for(int i = 0; i < *nodesLength; i++){

        if((*nodes)[i].stringCharacter.frequency < (*nodes)[minNodeIndex].stringCharacter.frequency)
            minNodeIndex = i;

    }

    // Nos creamos el nodo mínimo y le inicializamos los valores
    minNode = (TreeNode_s*)malloc(sizeof(TreeNode_s));
    minNode->leftChild = (*nodes)[minNodeIndex].leftChild;
    minNode->parentNode = (*nodes)[minNodeIndex].parentNode;
    minNode->rightChild = (*nodes)[minNodeIndex].rightChild;
    minNode->stringCharacter = (*nodes)[minNodeIndex].stringCharacter;

    // Cambiamos el nodo mínimo por el último para "sacarlo" del array y decrementamos la longitud del array
    (*nodes)[minNodeIndex] = (*nodes)[*nodesLength - 1];
    *nodesLength -= 1;

    return minNode;

}

// printTree
void printTree(FILE *file, TreeNode_s *tree){

    if(tree->leftChild != NULL){

        fprintf(file, "L\n");
        printTree(file, tree->leftChild);

    }
    else{
        
        fprintf(file, "%c\n", tree->stringCharacter.character);
        return;

    }

    if(tree->rightChild != NULL){

        fprintf(file, "R\n");
        printTree(file, tree->rightChild);

    }
    else{

        fprintf(file, "%c\n", tree->stringCharacter.character);
        return;

    }

}

// freeTree
void freeTree(TreeNode_s *tree){

    // Caso base (El nodo es una hoja)
    if(tree->leftChild == NULL && tree->rightChild == NULL)
        free(tree);
    // Si es una rama / raíz
    else{

        // Si tiene hijo izquierdo lo liberamos
        if(tree->leftChild != NULL)
            freeTree(tree->leftChild);
        
        // Si tiene hijo derecho lo liberamos
        if(tree->rightChild != NULL)
            freeTree(tree->rightChild);

    }

}

// initHuffmanCodes
HuffmanCode_s *initHuffmanCodes(){

    // Variables necesarias
    HuffmanCode_s *huffmanCodes = NULL;

    // Reservamos la memoria necesaria
    huffmanCodes = (HuffmanCode_s*)malloc(HASH_TABLE_SIZE * sizeof(HuffmanCode_s));

    // Inicializamos los códigos de huffman
    for(int i = 'a'; i <= 'z'; i++){

        huffmanCodes[getHash(i)].character = i;
        huffmanCodes[getHash(i)].code = NULL;
        huffmanCodes[getHash(i)].codeLength = 0;

    }

    for(int i = '0'; i <= '9'; i++){

        huffmanCodes[getHash(i)].character = i;
        huffmanCodes[getHash(i)].code = NULL;
        huffmanCodes[getHash(i)].codeLength = 0;

    }

    huffmanCodes[getHash(' ')].character = ' ';
    huffmanCodes[getHash(' ')].code = NULL;
    huffmanCodes[getHash(' ')].codeLength = 0;

    huffmanCodes[getHash(',')].character = ',';
    huffmanCodes[getHash(',')].code = NULL;
    huffmanCodes[getHash(',')].codeLength = 0;
    
    huffmanCodes[getHash('.')].character = '.';
    huffmanCodes[getHash('.')].code = NULL;
    huffmanCodes[getHash('.')].codeLength = 0;

    // Devolvemos la tabla hash de códigos huffman inicializada
    return huffmanCodes;

}

// generateHuffmanTree
void generateHuffmanCodes(HuffmanCode_s **huffmanCodes, TreeNode_s *huffmanTree, int *currentCode, int depth, int *maxDepth){

    // Variables necesarias
    int *currentCodeCopy = NULL;

    // Caso base (Es un nodo hoja)
    if(huffmanTree->leftChild == NULL && huffmanTree->rightChild == NULL){

        // Obtenemos el código Huffman del carácter 
        (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].character = huffmanTree->stringCharacter.character;
        (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].codeLength = depth;
        (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].code = (char*)malloc(sizeof(char));
        (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].code[0] = '0';

        // Si la profundidad es mayor que 0 (No es el único nodo) transformamos el valor del array en cadena de caracteres
        for(int i = 0; i < depth; i++){

            // Introducimos el caracter
            (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].code[i] = currentCode[i] + '0';

            // Aumentamos el tamaño del array
            (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].code = (char*)realloc(
                (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].code, 
                (i + 2) * sizeof(char)
            );

        }

        // Introducimos el final de cadena
        (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].code[depth] = '\0';

        // Comprobamos si la longitud del código (depth) es mayor que la mayor actual
        if(depth > *maxDepth)
            *maxDepth = depth;

        /* printf("CHAR: %c -> ARR: (", huffmanTree->stringCharacter.character);
        for(int i = 0; i < depth; i++)
            printf("%d, ", currentCode[i]);
        printf(") NUM: %u\n", (*huffmanCodes)[getHash(huffmanTree->stringCharacter.character)].value); */

    }
    // Si es un nodo rama / raíz
    else{

        // Nos copiamos el código actual
        currentCodeCopy = (int*)realloc(currentCodeCopy, (depth + 1) * sizeof(int));

        for(int i = 0; i < depth; i++)
            currentCodeCopy[i] = currentCode[i];

        // Si tiene hijos a la izquierda
        if(huffmanTree->leftChild != NULL){

            // Le agregamos el código 0
            currentCodeCopy[depth] = 0;

            // Obtenemos los códigos de la rama izquierda
            generateHuffmanCodes(huffmanCodes, huffmanTree->leftChild, currentCodeCopy, depth + 1, maxDepth);

        }

        // Si tiene hijos a la derecha
        if(huffmanTree->rightChild != NULL){

            // Le agregamos el código 1
            currentCodeCopy[depth] = 1;

            // Obtenemos los códigos de la rama derecha
            generateHuffmanCodes(huffmanCodes, huffmanTree->rightChild, currentCodeCopy, depth + 1, maxDepth);

        }

        // Liberamos la memoria utilizada
        free(currentCodeCopy);

    }

}

// printHuffmanCodes
void printHuffmanCodes(char *fileName, LinkedListNode_s *charactersList, HuffmanCode_s *huffmanCodes){

    // Variables necesarias
    FILE *file = NULL;
    LinkedListNode_s *charactersListCopy = NULL;

    // Abrimos el fichero
    file = fopen(fileName, "w");

    // Comprobamos que el fichero se haya abierto correctamente
    if(file == NULL){

        printf("ERROR: Ha ocurrido un error al intentar abrir el fichero '%s'.\n", fileName);
        exit(1);

    }

    // Realizamos una copia de la lista de caracteres
    charactersListCopy = charactersList;

    // Recorremos la lista de caracteres mostrando los códigos huffman
    while(charactersListCopy != NULL){

        // Variables necesarias
        char currentChar = '\0';

        // Obtenemos el caracter
        currentChar = charactersListCopy->stringCharacter.character;

        /* printf("%c -> %d\n", currentChar, huffmanCodes[getHash(currentChar)].value); */
        fprintf(file, "%c -> %s\n", currentChar, huffmanCodes[getHash(currentChar)].code);

        charactersListCopy = charactersListCopy->nextNode;

    }

    fclose(file);

}

// encodeFileContent
byte* encodeFileContent(FileContent_s fileContent, HuffmanCode_s *huffmanCodes, int maxCodeLength, int *bytesLength){

    // Variables necesarias
    byte *encodedFileContent = NULL;
    byte *encodedFileContentCopy = NULL;
    int charCounter = 0;
    int bitCounter = 0;
    byte auxByte = 0;

    // Calculamos la cantidad de caracteres a codificar
    for(int i = 0; i < fileContent.linesNumber; i++)
        charCounter += fileContent.fileLines[i].lineLength;

    // Reservamos memoria para la cadena codificada (El tamaño de los datos codificados y la cantidad de caracteres)
    // Si el número de bits es múltiplo del tamaño de un byte pedimos el número de bits entre el tamaño de un byte
    if(((charCounter * maxCodeLength) % (sizeof(byte) * BITS_IN_BYTE)) == 0)
        encodedFileContent = (byte*)malloc((charCounter * maxCodeLength / BITS_IN_BYTE) + sizeof(int));
    // Si no lo es, reservamos espacio para los bytes necesarios, más uno 
    else
        encodedFileContent = (byte*)malloc((charCounter * maxCodeLength / BITS_IN_BYTE) + 1 + sizeof(int));

    // Realizamos una copia del puntero del contenido codificado
    encodedFileContentCopy = encodedFileContent;

    // Introducimos la cantidad de caracteres
    memcpy(encodedFileContentCopy, &charCounter, sizeof(int));
    encodedFileContentCopy += sizeof(int);

    // Codificamos el contenido del fichero
    for(int i = 0; i < fileContent.linesNumber; i++){

        for(int j = 0; j < fileContent.fileLines[i].lineLength; j++){

            for(int k = 0; k < huffmanCodes[getHash(fileContent.fileLines[i].lineContent[j])].codeLength; k++){

                // Vamos introduciendo los bits del código huffman de la letra en el byte auxiliar (Si es 1 lo metemos y si es 0 no hace falta)
                if(huffmanCodes[getHash(fileContent.fileLines[i].lineContent[j])].code[k] == '1')
                    auxByte |= 0b1;
                
                bitCounter++;

                // Si llegamos al tamaño de un byte
                if(bitCounter >= BITS_IN_BYTE * sizeof(byte)){

                    // Copiamos el byte en el puntero, lo avanzamos a la siguiente posición y reiniciamos el contador y el byte auxiliar
                    memcpy(encodedFileContentCopy, &auxByte, sizeof(byte));
                    encodedFileContentCopy += sizeof(byte);
                    bitCounter = 0;
                    auxByte = 0;
                    *bytesLength += 1;

                }
                // Sino, avanzamos al siguiente bit
                else
                    auxByte <<= 1;

            }

            /* for(int k = maxCodeLength - 1; k >= 0; k--){

                // Vamos introduciendo los bits del código huffman de la letra en el byte auxiliar
                auxByte |= ((huffmanCodes[getHash(fileContent.fileLines[i].lineContent[j])].value >> k) & 0b1);
                bitCounter++;

                // Si llegamos al tamaño de un byte
                if(bitCounter >= BITS_IN_BYTE * sizeof(byte)){

                    // Copiamos el byte en el puntero, lo avanzamos a la siguiente posición y reiniciamos el contador y el byte auxiliar
                    memcpy(encodedFileContentCopy, &auxByte, sizeof(byte));
                    encodedFileContentCopy += sizeof(byte);
                    bitCounter = 0;
                    auxByte = 0;

                }
                // Sino, avanzamos al siguiente bit
                else
                    auxByte <<= 1;

            } */

        }

    }

    // Si nos quedan bits para llegar a un byte introducimos 0 hasta llegar al byte
    if(bitCounter != 0){

        auxByte <<= ((BITS_IN_BYTE * sizeof(byte)) - bitCounter);

        // Copiamos el byte en el puntero, lo avanzamos a la siguiente posición y reiniciamos el contador y el byte auxiliar
        memcpy(encodedFileContentCopy, &auxByte, sizeof(byte));
        encodedFileContentCopy += sizeof(byte);
        bitCounter = 0;
        auxByte = 0;
        *bytesLength += 1;

    }

    return encodedFileContent;

}

void printEncodedFileContent(char *fileName, byte *encodeFileContent, int length){

    // Variables necesarias
    FILE *file = NULL;

    // Abrimos el fichero
    file = fopen(fileName, "wb");

    // Comprobamos que el fichero se haya abierto correctamente
    if(file == NULL){

        printf("ERROR: Ha ocurrido un error al intentar abrir el fichero '%s'.\n", fileName);
        exit(1);

    }

    // Volcamos el contenido cifrado en el fichero
    printf("LEN: %d\n", length);
    fwrite(encodeFileContent, 1, length, file);

    // Cerramos el fichero
    fclose(file);

}

// initHashTable
HashTable_s* initHashTable(){

    // Variables necesarias
    HashTable_s *hashTable = NULL;

    // Reservamos la memoria necesaria
    hashTable = (HashTable_s*)malloc(HASH_TABLE_SIZE * sizeof(HashTable_s));

    // Inicializamos la tabla hash
    for(int i = 'a'; i <= 'z'; i++){

        hashTable[getHash(i)].key = i;
        hashTable[getHash(i)].value = 0;

    }

    for(int i = '0'; i <= '9'; i++){

        hashTable[getHash(i)].key = i;
        hashTable[getHash(i)].value = 0;

    }

    hashTable[getHash(' ')].key = ' ';
    hashTable[getHash(' ')].value = 0;

    hashTable[getHash(',')].key = ',';
    hashTable[getHash(',')].value = 0;

    hashTable[getHash('.')].key = '.';
    hashTable[getHash('.')].value = 0;;

    // Devolvemos la información
    return hashTable;

}

// getHash
int getHash(char key){

    // Variables necesarias
    int index = -1;

    // Calculamos el hash en función del carácter
    if(key >= 'a' && key <= 'z')
        index = key - 'a';
    else if(key >= 'A' && key <= 'Z')
        index = key - 'A';
    else if(key >= '0' && key <= '9')
        index = key - 22;
    else if(key == ' ')
        index = 36;
    else if(key == ',')
        index = 37;
    else if(key == '.')
        index = 38;

    return index;

}

// readLine
char *readLine(int *length){

    // Variables necesarias
    char *line = NULL;
    int lineLength = 0;
    char auxCharacter = '\0';

    // Inicializamos la cadena de caracteres
    line = (char*)malloc(sizeof(char));

    // Leemos de la entrada estandárd hasta que nos encontremos con el intro
    while((auxCharacter = getchar()) != '\n'){

        line[lineLength] = auxCharacter;
        lineLength++;
        line = (char*)realloc(line, (lineLength + 1) * sizeof(char));

    }

    line[lineLength] = '\0';

    // Devolvemos la información
    *length = lineLength;
    return line;

}

// readFileContent
FileContent_s readFileContent(char *fileName){

    // Variables necesarias
    FileContent_s fileContent;
    FILE *file = NULL;

    // Abrimos el fichero
    file = fopen(fileName, "r");

    // Comprobamos que el fichero se haya abierto correctamente
    if(file == NULL){

        printf("ERROR: El fichero no se ha podido abrir correctamente.\n");
        exit(1);

    }

    // Inicializamos el contenido del fichero
    fileContent.fileLines = NULL;
    fileContent.linesNumber = 0;

    // Mientras no nos encontremos con el final del fichero leemos línea a línea y la vamos introduciendo
    while(!feof(file)){

        fileContent.linesNumber += 1;
        fileContent.fileLines = (FileLine_s*)realloc(fileContent.fileLines, fileContent.linesNumber * sizeof(FileLine_s));
        fileContent.fileLines[fileContent.linesNumber - 1] = readFileLine(file);

    }

    // Cerramos el fichero
    fclose(file);

    // Devolvemos la información
    return fileContent;

}

// readFileLine
FileLine_s readFileLine(FILE *file){

    // Variables necesarias
    FileLine_s fileLine;
    char auxCharacter = '\0';

    // Comprobamos que el fichero se haya abierto correctamente
    if(file == NULL){

        printf("ERROR: No se ha podido abrir correctamente el fichero.\n");
        exit(1);

    }

    // Inicializamos las variables
    fileLine.lineLength = 0;
    fileLine.lineContent = (char*)malloc(sizeof(char));

    // Leemos la línea del fichero hasta que nos encontremos con un intro, o un final de fichero
    while ((auxCharacter = getc(file)) != '\n' && !feof(file))
    {
        
        fileLine.lineContent[fileLine.lineLength] = auxCharacter;
        fileLine.lineLength += 1;
        fileLine.lineContent = (char*)realloc(fileLine.lineContent, (fileLine.lineLength + 1) * sizeof(char));

    }

    // Introducimos el final de línea
    fileLine.lineContent[fileLine.lineLength] = '\0';

    // Devollvemos la información
    return fileLine;

}

// freeFileContent
void freeFileContent(FileContent_s fileContent){

    // Liberamos cada cadena de texto de cada línea
    for(int i = 0; i < fileContent.linesNumber; i++)
        free(fileContent.fileLines[i].lineContent);
    
    // Liberamos el puntero de líneas
    free(fileContent.fileLines);

}
