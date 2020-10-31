// Binary search tree. Define BALANCE to enable auto-balancing. This increases
// code size and node insert/delete time but ensures O(logN) searches.
#include <stdlib.h>
#include <stdbool.h>

struct node
{
    long long key;
    void * value;
    int counter;
    struct node *left, *right;
#ifdef BALANCE
    int height;
#endif
};

#ifdef BALANCE
// Return height of node from heights of subnodes
static int height (struct node *n)
{
    int l, r;
    if (!n) return 0;
    l = n->left ? n->left->height : 0;
    r = n->right ? n->right->height : 0;
    return ((l > r) ? l : r)+1;
}

// Rotate node right
//       A                  B
//      / \               /   \
//     B   d             C     A
//    / \     - - - >   / \   / \
//   C   c             a   b c   d
//  / \
// a   b
static struct node *ror(struct node *A)
{
    struct node *B = A->left;
    A->left = B->right;
    B->right = A;
    A->height = height(A);
    B->height = height(B);
    return B;
}

// Rotate node leftt
//   A                      B
//  / \                   /   \
// a   B                 A     C
//    / \     - - ->    / \   / \
//   b   C             a   b c   d
//      / \
//     c   d
static struct node *rol(struct node *A)
{
    struct node *B = A->right;
    A->right = B->left;
    B->left = A;
    A->height = height(A);
    B->height = height(B);
    return B;
}

// Return node's balance, <0 if leaning left, >0 if leaning right,  0 if
// balanced.
static long long balance(struct node *n)
{
    return n ? height(n->right) - height(n->left) : 0;
}
#endif

// Insert key and value into tree.
// If key already exists, just replace the value.
// Note *value must either be NULL or malloced.
// If counting then increment counter if key exists.
static struct node *insert(struct node *n, long long key, void *value, bool counting)
{
#ifdef BALANCE
    int bal;
#endif

    if (!n)
    {
        n = calloc(1,sizeof(struct node));
        if (!n) abort();
        n->key = key;
        n->value = value;
        n->counter = 1;
#ifdef BALANCE
        n->height = 1;
#endif
        return n;
    }

    if (key == n->key)
    {
        if (n->value) free(n->value);
        n->value = value;
        if (counting) n->counter++;
        return n;
    }

    if (key <= n->key)
        n->left = insert(n->left, key, value, counting);
    else
        n->right = insert(n->right, key, value, counting);

#ifdef BALANCE
    // re-balance
    n->height = height(n);
    bal = balance(n);
    if (bal < -1)
    {
        if (key < n->left->key) return ror(n);
        if (key >= n->left->key)
        {
            n->left = rol(n->left);
            return ror(n);
        }
    }
    else if (bal > 1)
    {
        if (key > n->right->key) return rol(n);
        if (key <= n->right->key)
        {
            n->right = ror(n->right);
            return rol(n);
        }
    }
#endif

    return n;
}

// Delete key from tree if it exists. If counting, only delete when node
// counter decrements to 0.
struct node *delete(struct node *n, long long key, bool counting)
{
#ifdef BALANCE
    int bal;
#endif

    if (!n) return NULL;

    if (key < n->key) n->left = delete(n->left, key, counting);
    else if (key > n->key) n->right = delete(n->right, key, counting);
    else
    {
        // ok, delete this node
        if (counting && --n->counter) return n; // maybe just decrement the count to 0

        if (n->value) free(n->value);

        if (!n->left && !n->right)
        {
            // no children
            free(n);
            return NULL;
        }
        if (!n->left || !n->right)
        {
            // one child
            struct node *t = n->left?:n->right;
            *n = *t; // memcpy
            free(t);
        }
        else
        {
            // two children, find the successor to this mode
            struct node *successor = n->right;
            while (successor->left) successor = successor->left;
            // clone successor's key, value, and counter
            n->key = successor->key;
            n->value = successor->value;
            n->counter = successor->counter;
            // then delete the succcessor
            successor->value = NULL;                    // there can only be one
            n->right = delete(n->right, n->key, false);
        }
    }

#ifdef BALANCE
    n->height = height(n);
    bal = balance(n);
    if (bal < -1)
    {
        if (balance(n->left) <= 0) return ror(n);
        n->left = rol(n->left);
        return ror(n);
    }

    if (bal > 1)
    {
        if (balance(n->right) >= 0) return rol(n);
        n->right = ror(n->right);
        return rol(n);
    }
#endif

    return n;
}

// Return node containing specified key, or NULL
struct node *search(struct node *n, long long key)
{
    while (n)
    {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else break;
    }
    return n;
}

// To build the proof-of-concept:   CFLAGS=-DPOC make -B bst
// To build the POC with balancing: CFLAGS="-DPOC -DBALANCE" make -B bst
#ifdef POC
#include <stdio.h>
#include <string.h>

// Cheap and dirty string hash
long long hash(char *key)
{
    long long h = 0;
    while (*key) h = (h * 33) + *key++;
    return h;
}

// All the animals live in a tree.
struct node *tree = NULL;

// If specified animal is in the tree report count, name, and sound.
// Otherwise, "Animal is a myth".
void say(char *animal)
{
    struct node *n = search(tree, hash(animal));
    if (!n) printf("The %s is a myth.\n", animal);
    else if (!n->value)
    {
        if (n->counter > 1) printf("%d %ss are quiet.\n", n->counter, animal);
        else printf("The %s is quiet.\n", animal);
    } else
    {
        if (n->counter > 1) printf("%d %ss say %s.\n", n->counter, animal, (char *)n->value);
        else printf("The %s says %s.\n", animal, (char *)n->value);
    }
}

// If animal not in tree, add it and set its sound (NULL is "quiet").
// Otherwise add another animal, leave existing sound as is NULL is given
void add(char *animal, char *sound)
{
    long long h = hash(animal);
    struct node *n;
    if (!sound && (n = search(tree, h)))
        // sound is NULL and animal exists, just increment it
        n -> counter++;
    else
        // else insert or update animal
        tree = insert(tree, h, sound ? strdup(sound) : NULL, true);
    say(animal);
}

// Add animal if not in tree, or just replace its sound.
void replace(char *animal, char *sound)
{
    tree = insert(tree, hash(animal), sound ? strdup(sound) : NULL, false);
    say(animal);
}

// Remove one of specified animal.
void kill(char *animal)
{
    tree = delete(tree, hash(animal), true);
    say(animal);
}

// Remove all of specified animals.
void extinct(char *animal)
{
    tree = delete(tree, hash(animal), false);
    say(animal); // should always be "a myth"
}

void dump(struct node *n)
{
    if (!n) return;
#ifdef BALANCE
    printf("%p: %d %lld %s <%p >%p\n", n, n->height, n->key, (char *)(n->value?:"NULL"), n->left, n->right);
#else
    printf("%p: %lld %s <%p >%p\n", n, n->key, (char *)(n->value?:"NULL"), n->left, n->right);
#endif
    dump(n->left);
    dump(n->right);
}

int main(int argc , char *argv[])
{
    add("ox", "moo");                   // "The ox says moo."
    add("cat","meow");                  // "The cat says meow."
    add("cow","moo");                   // "The cow says moo."
    add("cow",NULL);                     // "2 cows say moo."
    add("cow",NULL);                    // "3 cows say moo."
    add("dog","bark");                  // "The dog says bark."
    add("bird","chirp");                // "The bird says chirp."
    add("lion","mew");                  // "The lion says mew."
    add("horse","neigh");               // "The horse says neigh."
    add("pig","oink");                  // "The pig says oink."
    add("rabbit",NULL);                 // "The rabbit is quiet."
    add("rabbit",NULL);                 // "2 rabbits are quiet."
    replace("lion","roar");             // "The lion says roar."
    add("lion", NULL);                  // "2 lions say roar."
    add("lion", "growl");               // "3 lions say growl."
    replace("owl", "hoot");             // "The owl says hoot."
    say("unicorn");                     // "The unicorn is a myth."
    add("unicorn","tra-la tra-la");     // "The unicorn says tra-la tra-la."
    kill("cow");                        // "2 cows say moo."
    kill("lion");                       // "2 lions say growl."
    add("rabbit","I'M PETER DAMMIT");   // "3 rabbits say I'M PETER DAMMIT."
    kill("rabbit");                     // "2 rabbits say I'M PETER DAMMIT."
    kill("rabbit");                     // "The rabbit says I'M PETER DAMMIT."

    // Dump the tree, will be right-leaning if BALANCE not defined.
    dump(tree);

    kill("dog");                        // The dog is a myth.
    kill("owl");                        // The owl is a myth.
    kill("owl");                        // The owl is a myth.

    dump(tree);

    // Make valgrind happy
    kill("ox");                         // "The ox is a myth."
    kill("cat");                        // "The cat is a myth."
    extinct("cow");                     // "The cow is a myth."
    kill("bird");                       // "The bird is a myth."
    extinct("lion");                    // "The lion is a myth."
    kill("horse");                      // "The horse is a myth."
    kill("pig");                        // "The pig is a myth."
    kill("unicorn");                    // "The unicorn is a myth."
    kill("rabbit");                     // "The rabbit is a myth."

    dump(tree);                         // no output

    return 0;
}
#endif
