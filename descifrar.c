

/*
    Título: Descifrar.
    Nombre: Héctor Paredes Benavides
    Descripción: Creamos un programa que descifre el output del programa de cifrar
    Fecha: 19/12/2022
*/

/* Instrucciones de Preprocesado */
// Inclusión de bibliotecas externas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definición de constantes
#define TREE_FILE "tree.txt"
#define ENCODED_FILE "compressed.bin"

#define byte char
#define BITS_IN_BYTE 8

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

typedef struct BinFileContent_s{

    int length;
    byte *fileContent;

}BinFileContent_s;

typedef struct StringCharacter_s{

    char character;
    int frequency;

}StringCharacter_s;

typedef struct TreeNode_s{

    StringCharacter_s stringCharacter;
    struct TreeNode_s *parentNode;
    struct TreeNode_s *leftChild;
    struct TreeNode_s *rightChild;

}TreeNode_s;

// Prototipado de Funciones
// Funciones de Árboles
TreeNode_s* buildTreeFromFile(char *fileName);
void freeTree(TreeNode_s *tree);

// Funciones Huffman
char *decodeFileContent(BinFileContent_s fileContent, TreeNode_s *huffmanTree);

// Funciones auxiliares
FileContent_s readFileContent(char *fileName);
FileLine_s readFileLine(FILE *file);
void freeFileContent(FileContent_s fileContent);

// Funciones de ficheros binarios
BinFileContent_s readBinFile(char *fileName);

/* Función Principal Main */
int main(int argc, char **argv){

    // Variables necesarias
    TreeNode_s *huffmanTree = NULL;
    BinFileContent_s encodedFileContent;
    char *decodedContent = NULL;

    // Reconstruímos el árbol de Huffman
    huffmanTree = buildTreeFromFile(TREE_FILE);

    // Obtenemos el contenido del fichero cifrado
    encodedFileContent = readBinFile(ENCODED_FILE);

    // Desciframos el contenido del fichero
    decodedContent = decodeFileContent(encodedFileContent, huffmanTree);

    printf("Contenido: %s\n", decodedContent);

    // Liberamos la memoria utilizada
    freeTree(huffmanTree);

    free(encodedFileContent.fileContent);
    free(decodedContent);

    return 0;

}

/* Codificación de Funciones */
// buildTreeFromFile
TreeNode_s* buildTreeFromFile(char *fileName){

    // Variables necesarias
    FileContent_s treeFileContent;
    char currentChar = '\0';
    TreeNode_s *treeRoot = NULL;
    TreeNode_s *treeRootCopy = NULL;

    // Leemos el contenido del fichero
    treeFileContent = readFileContent(fileName);

    // Inicializamos el árbol
    treeRoot = (TreeNode_s*)malloc(sizeof(TreeNode_s));
    treeRoot->parentNode = NULL;
    treeRoot->leftChild = NULL;
    treeRoot->rightChild = NULL;
    treeRootCopy = treeRoot;

    // Reconstruímos el árbol de Huffman
    for(int i = 0; i < treeFileContent.linesNumber; i++){

        currentChar = treeFileContent.fileLines[i].lineContent[0];

        if(currentChar == 'L'){

            // Nos creamos un nuevo hijo izquierdo y nos movemos a él
            treeRootCopy->leftChild = (TreeNode_s*)malloc(sizeof(TreeNode_s));
            treeRootCopy->leftChild->parentNode = treeRootCopy;
            treeRootCopy = treeRootCopy->leftChild;

        }
        else if(currentChar == 'R'){

            // Vamos escalando el árbol buscando el primer nodo que no tiene hijo derecho
            while(treeRootCopy->rightChild != NULL){
                /* printf("SUBE\n"); */
                treeRootCopy = treeRootCopy->parentNode;
            }

            // Nos creamos el nuevo hijo derecho y nos movemos a él
            treeRootCopy->rightChild = (TreeNode_s*)malloc(sizeof(TreeNode_s));
            treeRootCopy->rightChild->parentNode = treeRootCopy;
            treeRootCopy = treeRootCopy->rightChild;

        }
        else if(currentChar){

            // Introducimos el carácter y nos vamos al nodo anterior
            treeRootCopy->stringCharacter.character = currentChar;
            treeRootCopy = treeRootCopy->parentNode;

        }

    }

    return treeRoot;

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

// decodeFileContent
char* decodeFileContent(BinFileContent_s fileContent, TreeNode_s *huffmanTree){

    // Variables necesarias
    char *decodedContent = NULL;
    int decodedContentLength = 0;
    byte *auxPointer = NULL;
    byte auxByte = '\0';
    int charactersNumber = 0;
    TreeNode_s *huffmanTreeCopy = NULL;

    // Inicializamos el contenido descifrado
    decodedContent = (char*)malloc(sizeof(char));

    // Inicializamos el puntero auxiliar al contenido del fichero
    auxPointer = fileContent.fileContent;

    // Obtenemos el número de caracteres de la cadena
    memcpy(&charactersNumber, auxPointer, sizeof(int));
    auxPointer += sizeof(int);

    // Inicializamos la copia del árbol de Huffman
    huffmanTreeCopy = huffmanTree;

    // Recorremos el resto de bytes descifrando la información
    for(int i = 0; i < fileContent.length - sizeof(int); i++){

        // Copiamos el byte en el byte auxiliar
        memcpy(&auxByte, auxPointer, sizeof(byte));
        auxPointer += sizeof(byte);

        // Recorremos los bits del byte de izquierda a derecha (más significativo a menos significativo)
        for(int j = BITS_IN_BYTE - 1; j >= 0; j--){

            // Si estamos en un nodo hoja leemos su valor y lo volcamos a la cadena descifrada
            if(huffmanTreeCopy->leftChild == NULL && huffmanTreeCopy->rightChild == NULL){


                decodedContent[decodedContentLength] = huffmanTreeCopy->stringCharacter.character;
                decodedContentLength++;
                decodedContent = (char*)realloc(decodedContent, (decodedContentLength + 1) * sizeof(char));

                // Volvemos al inicio del árbol para empezar a leer otro carácter
                huffmanTreeCopy = huffmanTree;

            }

            // Si nos tenemos que ir a la izquierda avanzamos el puntero a su hijo izquierdo
            if(((auxByte >> j) & 0b1) == 0)
                huffmanTreeCopy = huffmanTreeCopy->leftChild;
            // Si nos tenemos que ir a la derecha avanzamos el puntero a su hijo derecho
            else
                huffmanTreeCopy = huffmanTreeCopy->rightChild;

        }

    }

    // Insertamos el carácter de fin de cadena a la cadena con el contenido descifrado
    decodedContent[decodedContentLength] = '\0';

    return decodedContent;

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

// readBinFile
BinFileContent_s readBinFile(char *fileName){

    // Variables necesarias
    FILE *file = NULL;
    BinFileContent_s fileContent;
    byte auxByte = '\0';

    // Abrimos el fichero y comprobamos que no haya errores
    file = fopen(fileName, "rb");

    if(file == NULL){

        printf("ERROR: Ha ocurrido un error al intentar abrir el fichero '%s'", fileName);
        exit(1);

    }

    // Inicializamos el contenido del fichero
    fileContent.length = 0;
    fileContent.fileContent = NULL;

    // Leemos el contenido del fichero
    while((auxByte = fgetc(file)) != EOF){

        fileContent.fileContent = (byte*)realloc(fileContent.fileContent, (fileContent.length + 1) * sizeof(byte));
        fileContent.fileContent[fileContent.length] = auxByte;
        fileContent.length++;

    }

    // Cerramos el fichero
    fclose(file);

    return fileContent;

}
