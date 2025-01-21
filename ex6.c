#include "ex6.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define INT_BUFFER 128
# define DIRECTION_BUFFER_SIZE 3

// ================================================
// Basic struct definitions from ex6.h assumed:
//   PokemonData { int id; char *name; PokemonType TYPE; int hp; int attack; EvolutionStatus CAN_EVOLVE; }
//   PokemonNode { PokemonData* data; PokemonNode* left, *right; }
//   OwnerNode   { char* ownerName; PokemonNode* pokedexRoot; OwnerNode *next, *prev; }
//   OwnerNode* ownerHead;
//   const PokemonData pokedex[];
// ================================================

// --------------------------------------------------------------
// 1) Safe integer reading
// --------------------------------------------------------------

void trimWhitespace(char *str)
{
    // Remove leading spaces/tabs/\r
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r')
        start++;

    if (start > 0)
    {
        int idx = 0;
        while (str[start])
            str[idx++] = str[start++];
        str[idx] = '\0';
    }

    // Remove trailing spaces/tabs/\r
    int len = (int)strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r'))
    {
        str[--len] = '\0';
    }
}

char *myStrdup(const char *src)
{
    if (!src)
        return NULL;
    size_t len = strlen(src);
    char *dest = (char *)malloc(len + 1);
    if (!dest)
    {
        printf("Memory allocation failed in myStrdup.\n");
        return NULL;
    }
    strcpy(dest, src);
    return dest;
}

int readIntSafe(const char *prompt) {
    char buffer[INT_BUFFER];
    int value;
    int success = 0;

    while (!success) {
        printf("%s", prompt);

        // Read input
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Invalid input.\n");
            clearerr(stdin); // Clear the input stream error flag
            continue;
        }

        // Remove trailing newline or carriage return
        size_t len = strlen(buffer);
        if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[--len] = '\0';
        }

        // If the input is empty after trimming
        if (len == 0) {
            printf("Invalid input.\n");
            continue;
        }

        // Parse the input as an integer
        char *endptr;
        value = (int)strtol(buffer, &endptr, 10);

        // If the entire input was not consumed, it's invalid
        if (endptr == buffer || *endptr != '\0') {
            printf("Invalid input.\n");
        } else {
            success = 1; // Valid integer input
        }
    }
    return value;
}

PokemonNode *createPokemonNode(const PokemonData *data) {
    if (!data) return NULL;

    PokemonNode *newNode = (PokemonNode *)malloc(sizeof(PokemonNode));
    if (!newNode) {
        printf("Memory allocation failed for PokemonNode.\n");
        return NULL;
    }

    newNode->data = (PokemonData *)malloc(sizeof(PokemonData));
    if (!newNode->data) {
        printf("Memory allocation failed for PokemonData.\n");
        free(newNode);
        return NULL;
    }

    newNode->data->id = data->id;
    newNode->data->name = myStrdup(data->name); // Duplicate string
    newNode->data->TYPE = data->TYPE;
    newNode->data->hp = data->hp;
    newNode->data->attack = data->attack;
    newNode->data->CAN_EVOLVE = data->CAN_EVOLVE;

    newNode->left = newNode->right = NULL;

    return newNode;
}

OwnerNode *createOwner(char *ownerName, PokemonNode *starter) {
    if (!ownerName || !starter) return NULL;

    OwnerNode *newOwner = (OwnerNode *)malloc(sizeof(OwnerNode));
    if (!newOwner) {
        printf("Memory allocation failed for OwnerNode.\n");
        return NULL;
    }

    newOwner->ownerName = ownerName;
    newOwner->pokedexRoot = starter;
    newOwner->next = newOwner->prev = newOwner; // Self-loop for circular list

    return newOwner;
}


// --------------------------------------------------------------
// 2) Utility: Get type name from enum
// --------------------------------------------------------------
const char *getTypeName(PokemonType type)
{
    switch (type)
    {
    case GRASS:
        return "GRASS";
    case FIRE:
        return "FIRE";
    case WATER:
        return "WATER";
    case BUG:
        return "BUG";
    case NORMAL:
        return "NORMAL";
    case POISON:
        return "POISON";
    case ELECTRIC:
        return "ELECTRIC";
    case GROUND:
        return "GROUND";
    case FAIRY:
        return "FAIRY";
    case FIGHTING:
        return "FIGHTING";
    case PSYCHIC:
        return "PSYCHIC";
    case ROCK:
        return "ROCK";
    case GHOST:
        return "GHOST";
    case DRAGON:
        return "DRAGON";
    case ICE:
        return "ICE";
    default:
        return "UNKNOWN";
    }
}

// --------------------------------------------------------------
// Utility: getDynamicInput (for reading a line into malloc'd memory)
// --------------------------------------------------------------
char *getDynamicInput()
{
    char *input = NULL;
    size_t size = 0, capacity = 1;
    input = (char *)malloc(capacity);
    if (!input)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        if (size + 1 >= capacity)
        {
            capacity *= 2;
            char *temp = (char *)realloc(input, capacity);
            if (!temp)
            {
                printf("Memory reallocation failed.\n");
                free(input);
                return NULL;
            }
            input = temp;
        }
        input[size++] = (char)c;
    }
    input[size] = '\0';

    // Trim any leading/trailing whitespace or carriage returns
    trimWhitespace(input);

    return input;
}

// Function to print a single Pokemon node
void printPokemonNode(PokemonNode *node)
{
    if (!node)
        return;
    printf("ID: %d, Name: %s, Type: %s, HP: %d, Attack: %d, Can Evolve: %s\n",
           node->data->id,
           node->data->name,
           getTypeName(node->data->TYPE),
           node->data->hp,
           node->data->attack,
           (node->data->CAN_EVOLVE == CAN_EVOLVE) ? "Yes" : "No");
}
PokemonNode *insertPokemonNode(PokemonNode *root, PokemonNode *newNode) {
    if (!root) return newNode;

    if (newNode->data->id < root->data->id) {
        root->left = insertPokemonNode(root->left, newNode);
    } else if (newNode->data->id > root->data->id) {
        root->right = insertPokemonNode(root->right, newNode);
    } else {
        // Duplicate ID, free the new node
        freePokemonNode(newNode);
    }

    return root;
}

PokemonNode *searchPokemonBFS(PokemonNode *root, int id) {
    if (!root) return NULL;

    NodeArray queue;
    initNodeArray(&queue, 10);
    addNode(&queue, root);

    for (int i = 0; i < queue.size; i++) {
        PokemonNode *current = queue.nodes[i];
        if (current->data->id == id) {
            free(queue.nodes);
            return current;
        }

        if (current->left) addNode(&queue, current->left);
        if (current->right) addNode(&queue, current->right);
    }

    free(queue.nodes);
    return NULL;
}
void linkOwnerInCircularList(OwnerNode *newOwner) {
    if (!newOwner) return;

    if (!ownerHead) {
        ownerHead = newOwner;
        return;
    }

    OwnerNode *tail = ownerHead->prev;
    tail->next = newOwner;
    newOwner->prev = tail;
    newOwner->next = ownerHead;
    ownerHead->prev = newOwner;
}

OwnerNode *findOwnerByName(const char *name) {
    if (!ownerHead || !name) return NULL;

    OwnerNode *current = ownerHead;
    do {
        if (strcmp(current->ownerName, name) == 0) {
            return current;
        }
        current = current->next;
    } while (current != ownerHead);

    return NULL;
}

PokemonNode *removePokemonByID(PokemonNode *root, int id) {
    if (!root) return NULL;

    // Traverse the tree to find the node to delete
    if (id < root->data->id) {
        root->left = removePokemonByID(root->left, id);
        return root;
    }

    if (id > root->data->id) {
        root->right = removePokemonByID(root->right, id);
        return root;
    }

    // Node to delete found
    if (!root->left && !root->right) {
        // Case 1: Node has no children (leaf node)
        freePokemonNode(root);
        return NULL;
    }

    if (!root->left) {
        // Case 2: Node has only a right child
        PokemonNode *temp = root->right;
        freePokemonNode(root);
        return temp;
    }

    if (!root->right) {
        // Case 3: Node has only a left child
        PokemonNode *temp = root->left;
        freePokemonNode(root);
        return temp;
    }

    // Case 4: Node has two children
    // Find the in-order successor (smallest in the right subtree)
    PokemonNode *successor = root->right;
    while (successor->left) {
        successor = successor->left;
    }

    // Replace root's data with the successor's data
    free(root->data->name);
    root->data->name = myStrdup(successor->data->name);
    root->data->id = successor->data->id;
    root->data->TYPE = successor->data->TYPE;
    root->data->hp = successor->data->hp;
    root->data->attack = successor->data->attack;
    root->data->CAN_EVOLVE = successor->data->CAN_EVOLVE;

    // Remove the successor node
    root->right = removePokemonByID(root->right, successor->data->id);
    return root;
}
// --------------------------------------------------------------
// Display Menu
// --------------------------------------------------------------


void openPokedexMenu() {
    printf("Your name: ");
    char *ownerName = getDynamicInput();

    if (!ownerName || strlen(ownerName) > 20) {
        printf("Invalid name. Please try again.\n");
        free(ownerName);
        return;
    }

    if (findOwnerByName(ownerName) != NULL) {
        printf("Owner '%s' already exists. Not creating a new Pokedex.\n", ownerName);
        free(ownerName);
        return;
    }

    printf("Choose Starter:\n1. Bulbasaur\n2. Charmander\n3. Squirtle\n");
    int starterChoice = readIntSafe("Your choice: ");
    PokemonNode *starterNode = NULL;

    switch (starterChoice) {
    case 1:
        starterNode = createPokemonNode(&pokedex[0]);
        break;
    case 2:
        starterNode = createPokemonNode(&pokedex[3]);
        break;
    case 3:
        starterNode = createPokemonNode(&pokedex[6]);
        break;
    default:
        printf("Invalid choice. Please try again.\n");
        free(ownerName);
        return;
    }

    if (!starterNode) {
        printf("Failed to create the starter Pokémon.\n");
        free(ownerName);
        return;
    }

    OwnerNode *newOwner = createOwner(ownerName, starterNode);
    if (!newOwner) {
        printf("Failed to create the new owner.\n");
        free(ownerName);
        freePokemonNode(starterNode);
        return;
    }

    linkOwnerInCircularList(newOwner);
    printf("New Pokedex created for %s with starter %s.\n", ownerName, starterNode->data->name);
}
void addNode(NodeArray *na, PokemonNode *node) {
    if (!na || !node) return;

    // בדיקה אם המערך מלא
    if (na->size == na->capacity) {
        na->capacity *= 2;
        na->nodes = (PokemonNode **)realloc(na->nodes, na->capacity * sizeof(PokemonNode *));
        if (!na->nodes) {
            printf("Memory reallocation failed for NodeArray.\n");
            exit(1);
        }
    }
    na->nodes[na->size++] = node;
}

void displayBFS(PokemonNode *root) {
    if (!root) {
        printf("Pokedex is empty.\n");
        return;
    }
    BFSGeneric(root, printPokemonNode);
}

void preOrderTraversal(PokemonNode *root) {
    preOrderGeneric(root, printPokemonNode);
}

void inOrderTraversal(PokemonNode *root) {
    inOrderGeneric(root, printPokemonNode);
}

void postOrderTraversal(PokemonNode *root) {
    postOrderGeneric(root, printPokemonNode);
}

void displayAlphabetical(PokemonNode *root) {
    if (!root) {
        printf("Pokedex is empty.\n");
        return;
    }

    // אוספים את כל הצמתים
    NodeArray nodes;
    initNodeArray(&nodes, 10);
    collectAll(root, &nodes);

    // ממיינים לפי שם
    qsort(nodes.nodes, nodes.size, sizeof(PokemonNode *), compareByNameNode);

    // מדפיסים
    for (int i = 0; i < nodes.size; i++) {
        printPokemonNode(nodes.nodes[i]);
    }

    free(nodes.nodes);
}

void BFSGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root || !visit) return;

    // יצירת תור דינמי לצורך BFS
    NodeArray queue;
    initNodeArray(&queue, 10);

    // הוספת השורש לתור
    addNode(&queue, root);

    for (int i = 0; i < queue.size; i++) {
        PokemonNode *current = queue.nodes[i];

        // מבקר את הצומת הנוכחי
        visit(current);

        // הוספת ילדים לתור
        if (current->left) addNode(&queue, current->left);
        if (current->right) addNode(&queue, current->right);
    }

    free(queue.nodes);
}

int compareByNameNode(const void *a, const void *b) {
    PokemonNode *nodeA = *(PokemonNode **)a;
    PokemonNode *nodeB = *(PokemonNode **)b;

    return strcmp(nodeA->data->name, nodeB->data->name);
}

void preOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root || !visit) return;

    visit(root);
    preOrderGeneric(root->left, visit);
    preOrderGeneric(root->right, visit);
}

void inOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root || !visit) return;

    inOrderGeneric(root->left, visit);
    visit(root);
    inOrderGeneric(root->right, visit);
}

void postOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (!root || !visit) return;

    postOrderGeneric(root->left, visit);
    postOrderGeneric(root->right, visit);
    visit(root);
}
// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void enterExistingPokedexMenu() {
    if (!ownerHead) {
        printf("No existing Pokedexes.\n");
        return;
    }

    printf("\nExisting Pokedexes:\n");
    OwnerNode *current = ownerHead;
    int index = 1;

    // מציג את כל הבעלים
    do {
        printf("%d. %s\n", index++, current->ownerName);
        current = current->next;
    } while (current != ownerHead);

    int choice = readIntSafe("Choose a Pokedex by number:\n");

    if (choice < 1 || choice >= index) {
        printf("Invalid choice.\n");
        return;
    }

    // מוצא את הבעלים הנבחר
    current = ownerHead;
    for (int i = 1; i < choice; i++) {
        current = current->next;
    }

    printf("\nEntering %s's Pokedex...\n", current->ownerName);

    // תפריט פנימי לניהול הפוקדקס
    int subChoice;
    do {
        printf("\n-- %s's Pokedex Menu --\n", current->ownerName);
        printf("1. Add Pokemon\n");
        printf("2. Display Pokedex\n");
        printf("3. Release Pokemon (by ID)\n");
        printf("4. Pokemon Fight!\n");
        printf("5. Evolve Pokemon\n");
        printf("6. Back to Main\n");

        subChoice = readIntSafe("Your choice: ");

        switch (subChoice) {
            case 1:
                addPokemon(current);
                break;
            case 2:
                displayMenu(current);
                break;
            case 3:
                freePokemon(current);
                break;
            case 4:
                pokemonFight(current);
                break;
            case 5:
                evolvePokemon(current);
                break;
            case 6:
                printf("Back to Main Menu.\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (subChoice != 6);
}

// מממשים פונקציות נדרשות
void addPokemon(OwnerNode *owner) {
    printf("Enter ID to add: ");
    int id = readIntSafe("");

    // בדיקה אם הפוקימון כבר קיים
    PokemonNode *existing = searchPokemonBFS(owner->pokedexRoot, id);
    if (existing) {
        printf("Pokemon with ID %d is already in the Pokedex. No changes made.\n", id);
        return;
    }

    // יצירת פוקימון חדש והוספתו לעץ
    PokemonNode *newNode = createPokemonNode(&pokedex[id - 1]);
    owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, newNode);
    printf("Pokemon %s (ID %d) added.\n", pokedex[id - 1].name, id);
}

void displayMenu(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("Pokedex is empty.\n");
        return;
    }

    printf("Display:\n");
    printf("1. BFS (Level-Order)\n");
    printf("2. Pre-Order\n");
    printf("3. In-Order\n");
    printf("4. Post-Order\n");
    printf("5. Alphabetical (by name)\n");

    int choice = readIntSafe("Your choice: ");

    switch (choice) {
        case 1:
            displayBFS(owner->pokedexRoot);
            break;
        case 2:
            preOrderTraversal(owner->pokedexRoot);
            break;
        case 3:
            inOrderTraversal(owner->pokedexRoot);
            break;
        case 4:
            postOrderTraversal(owner->pokedexRoot);
            break;
        case 5:
            displayAlphabetical(owner->pokedexRoot);
            break;
        default:
            printf("Invalid choice.\n");
    }
}

void freePokemon(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("No Pokemon to release.\n");
        return;
    }

    printf("Enter Pokemon ID to release: "); // Updated prompt
    int id = readIntSafe("");

    // Find the Pokemon by ID
    PokemonNode *pokemon = searchPokemonBFS(owner->pokedexRoot, id);
    if (pokemon) {
        // Print the correct removal message
        printf("Removing Pokemon %s (ID %d).\n", pokemon->data->name, id);
        owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, id);
    } else {
        // Handle the case when the ID is not found
        printf("Pokemon with ID %d not found.\n", id);
    }
}

void pokemonFight(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("Pokedex is empty.\n");
        return;
    }

    printf("Enter ID of the first Pokemon: ");
    int id1 = readIntSafe("");

    printf("Enter ID of the second Pokemon: ");
    int id2 = readIntSafe("");

    PokemonNode *pokemon1 = searchPokemonBFS(owner->pokedexRoot, id1);
    PokemonNode *pokemon2 = searchPokemonBFS(owner->pokedexRoot, id2);

    if (!pokemon1 || !pokemon2) {
        printf("One or both Pokemon IDs not found.\n");
        return;
    }

    double score1 = pokemon1->data->attack * 1.5 + pokemon1->data->hp * 1.2;
    double score2 = pokemon2->data->attack * 1.5 + pokemon2->data->hp * 1.2;

    // Correctly formatted output with line breaks
    printf("Pokemon 1: %s (Score = %.2f)\n", pokemon1->data->name, score1);
    printf("Pokemon 2: %s (Score = %.2f)\n", pokemon2->data->name, score2);

    if (score1 > score2)
        printf("%s wins!\n", pokemon1->data->name);
    else if (score2 > score1)
        printf("%s wins!\n", pokemon2->data->name);
    else
        printf("It's a tie!\n");
}

void evolvePokemon(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("Cannot evolve. Pokedex empty.\n");
        return;
    }

    printf("Enter ID of Pokemon to evolve: ");
    int oldID = readIntSafe("");

    PokemonNode *pokemon = searchPokemonBFS(owner->pokedexRoot, oldID);
    if (!pokemon) {
        printf("No Pokemon with ID %d found.\n", oldID);
        return;
    }

    if (pokemon->data->CAN_EVOLVE == CANNOT_EVOLVE) {
        printf("Pokemon %s (ID %d) cannot evolve.\n", pokemon->data->name, oldID);
        return;
    }

    int newID = oldID + 1;

    // Check if the evolved form already exists
    PokemonNode *existing = searchPokemonBFS(owner->pokedexRoot, newID);
    if (existing) {
        printf("Evolved form already exists. Replacing old Pokemon.\n");
        owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, oldID);
        return;
    }

    // Create the new evolved Pokemon
    PokemonNode *newPokemon = createPokemonNode(&pokedex[newID - 1]);
    if (!newPokemon) {
        printf("Failed to create evolved Pokemon.\n");
        return;
    }

    // Update the tree
    owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, oldID);
    owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, newPokemon);

    printf("Pokemon evolved from %s (ID %d) to %s (ID %d).\n",
           pokemon->data->name, oldID, newPokemon->data->name, newID);
}

void deletePokedex() {
    if (!ownerHead) {
        printf("No existing Pokedexes to delete.\n");
        return;
    }

    printf("\n=== Delete a Pokedex ===\n");

    // Display all owners
    OwnerNode *current = ownerHead;
    int index = 1;

    do {
        printf("%d. %s\n", index++, current->ownerName);
        current = current->next;
    } while (current != ownerHead);

    int choice = readIntSafe("Choose a Pokedex to delete by number: ");

    if (choice < 1 || choice >= index) {
        printf("Invalid choice.\n");
        return;
    }

    // Find the selected owner
    current = ownerHead;
    for (int i = 1; i < choice; i++) {
        current = current->next;
    }

    printf("Deleting %s's entire Pokedex...\n", current->ownerName);

    // Update the circular doubly linked list
    if (current->next == current && current->prev == current) {
        // Only one owner in the list
        ownerHead = NULL;
    } else {
        // More than one owner
        current->prev->next = current->next;
        current->next->prev = current->prev;
        if (current == ownerHead) {
            ownerHead = current->next; // Update head if necessary
        }
    }

    // Free the memory
    freeOwnerNode(current);

    printf("Pokedex deleted.\n");
}

/**
 * @brief Free an OwnerNode (including name and entire Pokedex BST).
 * @param owner pointer to the OwnerNode to free
 */
void freeOwnerNode(OwnerNode *owner) {
    if (!owner) return;

    // Free the Pokedex BST
    freePokemonTree(owner->pokedexRoot);

    // Free the owner's name
    free(owner->ownerName);

    // Free the owner node itself
    free(owner);
}

/**
 * @brief Recursively free a BST of PokemonNodes.
 * @param root pointer to the root of the BST
 */
void freePokemonTree(PokemonNode *root) {
    if (!root) return;

    freePokemonTree(root->left);
    freePokemonTree(root->right);

    freePokemonNode(root);
}

/**
 * @brief Free a single PokemonNode, including its data.
 * @param node pointer to the PokemonNode to free
 */
void freePokemonNode(PokemonNode *node) {
    if (!node) return;

    // Free the Pokemon's name
    free(node->data->name);

    // Free the PokemonData struct
    free(node->data);

    // Free the node itself
    free(node);
}
void mergePokedexMenu() {
    // Check if there are enough owners to merge
    if (!ownerHead || ownerHead->next == ownerHead) {
        printf("Not enough owners to merge.\n");
        return;
    }

    printf("\n=== Merge Pokedexes ===\n");

    // Get the first owner's name
    printf("Enter name of first owner: ");
    char *firstOwnerName = getDynamicInput();

    OwnerNode *firstOwner = findOwnerByName(firstOwnerName);
    if (!firstOwner) {
        printf("Owner '%s' not found.\n", firstOwnerName);
        free(firstOwnerName);
        return;
    }

    // Get the second owner's name
    printf("Enter name of second owner: ");
    char *secondOwnerName = getDynamicInput();

    OwnerNode *secondOwner = findOwnerByName(secondOwnerName);
    if (!secondOwner) {
        printf("Owner '%s' not found.\n", secondOwnerName);
        free(firstOwnerName);
        free(secondOwnerName);
        return;
    }

    printf("Merging %s and %s...\n", firstOwnerName, secondOwnerName);

    // Merge the second owner's Pokedex into the first owner's Pokedex
    mergePokedexes(firstOwner->pokedexRoot, secondOwner->pokedexRoot);

    // Remove the second owner
    removeOwnerFromCircularList(secondOwner);

    printf("Merge completed.\nOwner '%s' has been removed after merging.\n", secondOwnerName);

    free(firstOwnerName);
    free(secondOwnerName);
}

/**
 * @brief Merges the second BST into the first BST.
 * @param firstRoot Pointer to the root of the first owner's BST
 * @param secondRoot Pointer to the root of the second owner's BST
 */
void mergePokedexes(PokemonNode *firstRoot, PokemonNode *secondRoot) {
    if (!secondRoot) return;

    // Perform BFS on the second BST
    NodeArray nodes;
    initNodeArray(&nodes, 10);
    collectAll(secondRoot, &nodes);

    for (int i = 0; i < nodes.size; i++) {
        PokemonNode *newNode = createPokemonNode(nodes.nodes[i]->data);
        firstRoot = insertPokemonNode(firstRoot, newNode);
    }

    free(nodes.nodes); // Free the dynamic array used for BFS
}

/**
 * @brief Removes an owner from the circular linked list.
 * @param target The owner to remove
 */
void removeOwnerFromCircularList(OwnerNode *target) {
    if (!target) return;

    // If the list has only one owner
    if (target->next == target && target->prev == target) {
        ownerHead = NULL;
    } else {
        // Update the neighbors to skip the target
        target->prev->next = target->next;
        target->next->prev = target->prev;

        if (target == ownerHead) {
            ownerHead = target->next; // Update head if necessary
        }
    }

    freeOwnerNode(target);
}

/**
 * @brief Initializes a NodeArray structure.
 * @param na Pointer to the NodeArray to initialize
 * @param cap Initial capacity of the array
 */
void initNodeArray(NodeArray *na, int cap) {
    na->nodes = (PokemonNode **)malloc(sizeof(PokemonNode *) * cap);
    if (!na->nodes) {
        printf("Memory allocation failed for NodeArray.\n");
        exit(1);
    }
    na->size = 0;
    na->capacity = cap;
}

/**
 * @brief Collects all nodes from a BST into a NodeArray.
 * @param root Pointer to the root of the BST
 * @param na Pointer to the NodeArray
 */
void collectAll(PokemonNode *root, NodeArray *na) {
    if (!root) return;

    // Dynamically expand the array if necessary
    if (na->size == na->capacity) {
        na->capacity *= 2;
        na->nodes = (PokemonNode **)realloc(na->nodes, sizeof(PokemonNode *) * na->capacity);
        if (!na->nodes) {
            printf("Memory reallocation failed for NodeArray.\n");
            exit(1);
        }
    }

    // Add the current node
    na->nodes[na->size++] = root;

    // Recurse left and right
    collectAll(root->left, na);
    collectAll(root->right, na);
}

void sortOwners() {
    if (!ownerHead || ownerHead->next == ownerHead) {
        // No owners or only one owner in the list
        printf("0 or 1 owners only => no need to sort.\n");
        return;
    }

    int swapped;
    OwnerNode *current;
    OwnerNode *last = NULL;

    // Bubble Sort for circular doubly linked list
    do {
        swapped = 0;
        current = ownerHead;

        while (current->next != last && current->next != ownerHead) {
            if (strcmp(current->ownerName, current->next->ownerName) > 0) {
                // Swap data between the two nodes
                swapOwnerData(current, current->next);
                swapped = 1;
            }
            current = current->next;
        }
        last = current;
    } while (swapped);

    printf("Owners sorted by name.\n");
}

/**
 * @brief Swaps the data of two OwnerNodes.
 * @param a First OwnerNode
 * @param b Second OwnerNode
 */
void swapOwnerData(OwnerNode *a, OwnerNode *b) {
    if (!a || !b) return;

    // Swap owner names
    char *tempName = a->ownerName;
    a->ownerName = b->ownerName;
    b->ownerName = tempName;

    // Swap Pokedex roots
    PokemonNode *tempPokedex = a->pokedexRoot;
    a->pokedexRoot = b->pokedexRoot;
    b->pokedexRoot = tempPokedex;
}

void printOwnersCircular() {
    if (!ownerHead) {
        printf("No owners.\n");
        return;
    }

    // Get the direction
    printf("Enter direction (F or B): ");
    char directionBuffer[DIRECTION_BUFFER_SIZE];
    fgets(directionBuffer, sizeof(directionBuffer), stdin);
    char direction = tolower(directionBuffer[0]); // Normalize to lowercase

    // Get the number of prints
    int times = readIntSafe("How many prints? ");

    OwnerNode *current = ownerHead;
    for (int i = 1; i <= times; i++) {
        printf("[%d] %s\n", i, current->ownerName);

        // Move in the chosen direction
        if (direction == 'f') {
            current = current->next;
        } else if (direction == 'b') {
            current = current->prev;
        }
    }
}
void freeAllOwners() {
    if (!ownerHead) return;

    OwnerNode *current = ownerHead;
    do {
        OwnerNode *next = current->next;
        freeOwnerNode(current);
        current = next;
    } while (current != ownerHead);

    ownerHead = NULL;
}

void mainMenu() {
    int choice;
    do {
        printf("\n=== Main Menu ===\n");
        printf("1. New Pokedex\n");
        printf("2. Existing Pokedex\n");
        printf("3. Delete a Pokedex\n");
        printf("4. Merge Pokedexes\n");
        printf("5. Sort Owners by Name\n");
        printf("6. Print Owners in a direction X times\n");
        printf("7. Exit\n");

        choice = readIntSafe("Your choice: ");

        switch (choice) {
        case 1:
            openPokedexMenu();
            break;
        case 2:
            enterExistingPokedexMenu();
            break;
        case 3:
            deletePokedex();
            break;
        case 4:
            mergePokedexMenu(); // Task 4
            break;
        case 5:
            sortOwners();
            break;
        case 6:
            printOwnersCircular();
            break;
        case 7:
            printf("Goodbye!\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 7);
    freeAllOwners();
}

int main() {
    mainMenu(); // Call the main menu function
    return 0;   // Return 0 to indicate successful execution
}